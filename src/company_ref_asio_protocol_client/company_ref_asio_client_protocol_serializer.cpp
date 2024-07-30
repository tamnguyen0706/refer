/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_client_protocol_serializer.cpp
 @brief Client Protocol Serializer MsgHandler
 */
#include "company_ref_asio_client_protocol_serializer.h"
#include "company_ref_asio_client_protocol_handler.h"

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_framing.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_utils/company_ref_weak_bind.h>
#include <Compan_logger/Compan_logger.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/post.hpp>

using namespace Compan::Edge;

CompanLogger ClientProtocolSerializerLog("client.protocol.serializer", LogLevel::Information);

ClientProtocolSerializer::ClientProtocolSerializer(
        boost::asio::io_context& ctx,
        ClientProtocolHandlerPtr clientProtocolHandler,
        uint32_t connectionId)
    : ctx_(ctx)
    , connectionId_(connectionId)
    , clientProtocolHandler_(clientProtocolHandler)
    , bNewFraming_(true)
    , pAecCallbacks_(new AecCallbacks<std::string>(
              {[]() { ErrorLog(ClientProtocolSerializerLog) << "Parse error..." << std::endl; },
               [this](AECFrameId const& frameId, std::string::const_iterator begin, std::string::const_iterator end) {
                   DebugLog(ClientProtocolSerializerLog) << "Received AEC frame with id " << frameId.str()
                                                         << " and size " << std::distance(begin, end) << std::endl;
                   if ((frameId == AECv09::frameId) || (frameId == AECv10::frameId)) {
                       onDecodeClientMessage(&(*begin), std::distance(begin, end));
                   }
               }}))
{
    FunctionLog(ClientProtocolSerializerLog);
}

ClientProtocolSerializer::~ClientProtocolSerializer() //= default;
{
    FunctionArgLog(ClientProtocolSerializerLog) << __FUNCTION__ << " [" << connectionId_ << "]" << std::endl;
}

void ClientProtocolSerializer::start()
{
    onEncodeServerMessage_ = clientProtocolHandler_->connectSendCallback(
            WeakBind(&ClientProtocolSerializer::onEncodeServerMessage, shared_from_this(), std::placeholders::_1));
}

void ClientProtocolSerializer::stop()
{
    onEncodeServerMessage_.disconnect();
    clientProtocolHandler_->disconnect();
}

void ClientProtocolSerializer::recvData(BufferType&& buffer)
{
    FunctionArgLog(ClientProtocolSerializerLog) << " [" << connectionId_ << "]" << std::endl;

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

void ClientProtocolSerializer::onDecodeClientMessage(char const* data, size_t const len)
{
    FunctionArgLog(ClientProtocolSerializerLog) << " [" << connectionId_ << "]" << std::endl;

    if (!data || len == 0) return;

    if (!isConnected()) return;

    ClientMessagePtr msgPtr = std::make_shared<CompanEdgeProtocol::ClientMessage>();
    if (!msgPtr->ParseFromArray(data, len)) {
        ErrorLog(ClientProtocolSerializerLog)
                << "[" << connectionId_ << "] onDecodeServerMessage, parse error" << std::endl;
        return;
    }

    boost::asio::post(ctx_, WeakBind(&ClientProtocolHandler::doHandleMessage, clientProtocolHandler_, msgPtr));
}

void ClientProtocolSerializer::onEncodeServerMessage(ServerMessagePtr msgPtr)
{
    FunctionArgLog(ClientProtocolSerializerLog) << " [" << connectionId_ << "]" << std::endl;

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
            ErrorLog(ClientProtocolSerializerLog)
                    << "[" << connectionId_ << "] failed to serialize : " << *msgPtr << std::endl;
            return;
        }

        buffer.resize(hdrSize + pbSize);
    } else {
        std::string hdr(std::to_string(pbSize) + ' ');
        std::size_t hdrSize = hdr.length();
        std::move(std::cbegin(hdr), std::cend(hdr), std::begin(buffer));

        if (!msgPtr->SerializeToArray(&buffer[hdrSize], pbSize)) {
            ErrorLog(ClientProtocolSerializerLog)
                    << "[" << connectionId_ << "] failed to serialize : " << *msgPtr << std::endl;
            return;
        }

        buffer.resize(hdrSize + pbSize);
    }

    sendData(std::move(buffer));
}
