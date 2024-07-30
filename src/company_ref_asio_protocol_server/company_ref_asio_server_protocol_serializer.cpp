/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_server_protocol_serializer.cpp
 @brief Server Protocol Serializer MsgHandler
 */
#include "company_ref_asio_server_protocol_serializer.h"

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_framing.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_utils/company_ref_weak_bind.h>
#include <Compan_logger/Compan_logger.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/post.hpp>

using namespace Compan::Edge;

CompanLogger ServerProtocolSerializerLog("server.protocol.serializer", LogLevel::Information);

ServerProtocolSerializer::ServerProtocolSerializer(
        boost::asio::io_context& ctx,
        VariantValueStore& ws,
        DmoContainer& dmo,
        std::mutex& wsContainerMutex,
        uint32_t connectionId)
    : ctx_(ctx)
    , connectionId_(connectionId)
    , serverProtocolHandler_(std::make_shared<ServerProtocolHandler>(ws, dmo, wsContainerMutex, connectionId))
    , bNewFraming_(true)
    , pAecCallbacks_(new AecCallbacks<std::string>(
              {[]() { ErrorLog(ServerProtocolSerializerLog) << "Parse error..." << std::endl; },
               [this](AECFrameId const& frameId, std::string::const_iterator begin, std::string::const_iterator end) {
                   DebugLog(ServerProtocolSerializerLog) << "Received AEC frame with id " << frameId.str()
                                                         << " and size " << std::distance(begin, end) << std::endl;
                   if ((frameId == AECv09::frameId) || (frameId == AECv10::frameId)) {
                       onDecodeServerMessage(&(*begin), std::distance(begin, end));
                   }
               }}))
{
    FunctionLog(ServerProtocolSerializerLog);
}

ServerProtocolSerializer::~ServerProtocolSerializer() //= default;
{
    FunctionArgLog(ServerProtocolSerializerLog) << __FUNCTION__ << " [" << connectionId_ << "]" << std::endl;
}

void ServerProtocolSerializer::start()
{
    onEncodeClientMessage_ = serverProtocolHandler_->connectSendCallback(
            WeakBind(&ServerProtocolSerializer::onEncodeClientMessage, shared_from_this(), std::placeholders::_1));
}

void ServerProtocolSerializer::stop()
{
    onEncodeClientMessage_.disconnect();
    serverProtocolHandler_->disconnect();
}

void ServerProtocolSerializer::recvData(BufferType&& buffer)
{
    FunctionArgLog(ServerProtocolSerializerLog) << " [" << connectionId_ << "]" << std::endl;

    if (!isConnected()) return;

    std::lock_guard<std::mutex> lock(bufferLock_);

    buffer_.insert(std::end(buffer_), std::begin(buffer), std::end(buffer));

    if (buffer_.empty()) return;

    if (!isConnected()) return;

    while (buffer_.size()) {
        if (buffer_[0] == AECv10::cSeparator) {
            bNewFraming_ = true;
            if (!AECv10::parseFrame(buffer_, *pAecCallbacks_)) break;
        } else {
            bNewFraming_ = false;
            if (!AECv09::parseFrame(buffer_, *pAecCallbacks_)) break;
        }
    }
}

void ServerProtocolSerializer::onDecodeServerMessage(char const* data, size_t const len)
{
    FunctionArgLog(ServerProtocolSerializerLog) << " [" << connectionId_ << "]" << std::endl;

    if (!data || len == 0) return;

    if (!isConnected()) return;

    ServerMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ServerMessage>();
    if (!msgPtr->ParseFromArray(data, len)) {
        ErrorLog(ServerProtocolSerializerLog)
                << "[" << connectionId_ << "] onDecodeServerMessage, parse error" << std::endl;
        return;
    }

    boost::asio::post(ctx_, WeakBind(&ServerProtocolHandler::doHandleMessage, serverProtocolHandler_, msgPtr));
}

void ServerProtocolSerializer::onEncodeClientMessage(ClientMessagePtr msgPtr)
{
    FunctionArgLog(ServerProtocolSerializerLog) << " [" << connectionId_ << "]" << std::endl;

    /// serialize the message back
    if (!isConnected() || !msgPtr) return;

    std::size_t pbSize = msgPtr->ByteSizeLong();

    if (!pbSize) return;

    BufferType buffer(pbSize + 64);

    if (bNewFraming_) {
        std::string hdr(AECv10::makeHeader<std::string>(pbSize));
        std::size_t hdrSize = hdr.length();
        std::move(cbegin(hdr), cend(hdr), buffer.begin());
        if (!msgPtr->SerializeToArray(&buffer[hdrSize], pbSize)) {
            ErrorLog(ServerProtocolSerializerLog)
                    << "[" << connectionId_ << "] failed to serialize : " << *msgPtr << std::endl;
            return;
        }

        buffer.resize(hdrSize + pbSize);
    } else {
        std::string hdr(std::to_string(pbSize) + ' ');
        std::size_t hdrSize = hdr.length();
        std::move(std::cbegin(hdr), std::cend(hdr), std::begin(buffer));

        if (!msgPtr->SerializeToArray(&buffer[hdrSize], pbSize)) {
            ErrorLog(ServerProtocolSerializerLog)
                    << "[" << connectionId_ << "] failed to serialize : " << *msgPtr << std::endl;
            return;
        }

        buffer.resize(hdrSize + pbSize);
    }

    sendData(std::move(buffer));
}
