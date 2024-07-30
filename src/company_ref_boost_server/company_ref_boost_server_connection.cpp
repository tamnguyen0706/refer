/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_server_connection.cpp
  @brief CompanEdge Boost Server - connection
*/

#include "company_ref_boost_server_connection.h"
#include "company_ref_boost_message_handler.h"
#include "company_ref_boost_server_connection_manager.h"

#include <company_ref_protocol_utils/company_ref_framing.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_protocol_utils/protobuf.h>
#include <Compan_logger/Compan_logger.h>

#include <boost/asio/completion_condition.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

#include <utility>

namespace Compan{
namespace Edge {
CompanLogger AebServerConnectionLog("server.connection.core", LogLevel::Information);
} // namespace Edge
} // namespace Compan

namespace Compan{
namespace Edge {

template <typename T>
CompanEdgeBoostServerConnection<T>::CompanEdgeBoostServerConnection(
        uint32_t connectionId,
        boost::asio::io_context& ioContext,
        boost::asio::basic_stream_socket<T> socket,
        CompanEdgeBoostMessageHandlerFactory& handlerFactory)
    : ioContext_(ioContext)
    , onDisconnectSignal_(ioContext_)
    , socket_(std::move(socket))
    , connectionId_(connectionId)
    , messageHandlerPtr_(handlerFactory.makeHandler(connectionId))
    , bNewFraming_(true)
    , pAecCallbacks_(new AecCallbacks<std::string>(
              {[this]() {
                   DebugLog(AebServerConnectionLog) << "Parse error, closing connection..." << std::endl;
                   doClose();
               },
               [this](AECFrameId const& frameId, std::string::const_iterator begin, std::string::const_iterator end) {
                   DebugLog(AebServerConnectionLog) << "Received AEC frame with id " << frameId.str() << " and size "
                                                    << std::distance(begin, end) << std::endl;
                   if ((frameId == AECv09::frameId) || (frameId == AECv10::frameId)) {
                       parseRequestMessage(&(*begin), std::distance(begin, end));
                   } else
                       sendUnknownFrameId(frameId);
               }}))
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "]" << std::endl;
}

template <typename T>
CompanEdgeBoostServerConnection<T>::~CompanEdgeBoostServerConnection()
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "]" << std::endl;
}

template <typename T>
void CompanEdgeBoostServerConnection<T>::start()
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "]" << std::endl;

    // do bind withe shared_from_this() to keep the objects alive
    //  make sure to call CompanEdgeBoostMessageHandler::diconnect on tear down
    messageHandlerConnection_ = messageHandlerPtr_->connectClientMessageListener(std::bind(
            &CompanEdgeBoostServerConnection::sendClientMessage, this->shared_from_this(), std::placeholders::_1));

    doRead();
}

template <typename T>
void CompanEdgeBoostServerConnection<T>::stop()
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "]" << std::endl;

    if (socket_.is_open()) socket_.close();

    // dump queues to stop the post handlers
    {
        std::lock_guard<std::mutex> lock(rspMsgMutex_);
        rspMsgQueue_ = std::queue<ClientMessagePtr>();
    }

    {
        std::lock_guard<std::mutex> lock(serverMutex_);

        messageHandlerConnection_.disconnect();

        if (messageHandlerPtr_) messageHandlerPtr_->disconnect();

        messageHandlerPtr_.reset();

        serverQueue_ = std::queue<CompanEdgeProtocol::ServerMessage>();
    }
}

template <typename T>
void CompanEdgeBoostServerConnection<T>::sendClientMessage(ClientMessagePtr rspMsg)
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "]" << std::endl;

    if (!rspMsg) {
        ErrorLog(AebServerConnectionLog) << "[" << connectionId_ << "] rspMsg empty?" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(rspMsgMutex_);
    rspMsgQueue_.push(rspMsg);

    // if it's the FIRST entry, kick of writing
    //  doWrite burns down the queue
    if (rspMsgQueue_.size() == 1)
        boost::asio::post(ioContext_, std::bind(&CompanEdgeBoostServerConnection::doWrite, this->shared_from_this()));
}

template <typename T>
void CompanEdgeBoostServerConnection<T>::sendUnknownFrameId(AECFrameId const& frameId)
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "]" << std::endl;

    const std::string rspBuffer = AECv10::makeUnknownFrameId<std::string>(frameId);

    boost::system::error_code ec;

    boost::asio::write(socket_, boost::asio::buffer(rspBuffer.data(), rspBuffer.size()), ec);
    if (ec) {
        ErrorLog(AebServerConnectionLog) << "[" << connectionId_ << "] write payload failed, closing: " << ec.message()
                                         << std::endl;
        return doClose();
    }
}

template <typename T>
void CompanEdgeBoostServerConnection<T>::doWrite()
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "]" << std::endl;

    ClientMessagePtr rspMsg;

    {
        std::lock_guard<std::mutex> lock(rspMsgMutex_);

        if (rspMsgQueue_.empty()) {
            DebugLog(AebServerConnectionLog) << "[" << connectionId_ << "] responseMessage Q is empty" << std::endl;
            return;
        }

        rspMsg = rspMsgQueue_.front();
        rspMsgQueue_.pop();
    }

    if (!rspMsg) {

        ErrorLog(AebServerConnectionLog) << "[" << connectionId_ << "] rspMsg empty?" << std::endl;

        std::lock_guard<std::mutex> lock(rspMsgMutex_);
        if (!rspMsgQueue_.empty())
            boost::asio::post(ioContext_, std::bind(&CompanEdgeBoostServerConnection::doWrite, this->shared_from_this()));
        return;
    }

    std::string rspBuffer;
    if (!rspMsg->SerializeToString(&rspBuffer)) {
        ErrorLog(AebServerConnectionLog) << "[" << connectionId_ << "] failed to serialize : " << *rspMsg << std::endl;

        std::lock_guard<std::mutex> lock(rspMsgMutex_);
        if (!rspMsgQueue_.empty())
            boost::asio::post(ioContext_, std::bind(&CompanEdgeBoostServerConnection::doWrite, this->shared_from_this()));
        return;
    }

    if (bNewFraming_) {
        rspBuffer.insert(0, AECv10::makeHeader<std::string>(rspBuffer.cbegin(), rspBuffer.cend()));
    } else {
        rspBuffer.insert(0, AECv09::makeHeader<std::string>(rspBuffer.cbegin(), rspBuffer.cend()));
    }

    boost::system::error_code ec;

    boost::asio::write(socket_, boost::asio::buffer(rspBuffer.data(), rspBuffer.size()), ec);
    if (ec) {
        ErrorLog(AebServerConnectionLog) << "[" << connectionId_ << "] write payload failed, closing: " << ec.message()
                                         << std::endl;
        return doClose();
    }

    boost::asio::post(ioContext_, std::bind(&CompanEdgeBoostServerConnection::doWrite, this->shared_from_this()));
}

template <typename T>
void CompanEdgeBoostServerConnection<T>::doRead()
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "]" << std::endl;

    boost::asio::async_read(
            socket_,
            boost::asio::dynamic_buffer(readBuffer_),
            boost::asio::transfer_at_least(1),
            std::bind(
                    &CompanEdgeBoostServerConnection::onRead,
                    this->shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2));
}

template <typename T>
void CompanEdgeBoostServerConnection<T>::onRead(boost::system::error_code const& ec, size_t const length)
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "] len=" << length << ", ec=" << ec << std::endl;

    if (ec) {
        DebugLog(AebServerConnectionLog) << "[" << connectionId_ << "] Closing connection" << std::endl;
        return doClose();
    }

    if (length == 0) {
        ErrorLog(AebServerConnectionLog) << "[" << connectionId_
                                         << "] onHeader, parse error - closing connection, len:" << length << std::endl;
        return doClose();
    }

    while (!readBuffer_.empty()) {
        if (readBuffer_[0] == AECv10::cSeparator) {
            bNewFraming_ = true;
            if (!AECv10::parseFrame(readBuffer_, *pAecCallbacks_)) break;
        } else {
            bNewFraming_ = false;
            if (!AECv09::parseFrame(readBuffer_, *pAecCallbacks_)) break;
        }
    }

    // don't get smart and try to put this at the top
    //  * we aren't ready for scatter gather - yet
    doRead();
}

template <typename T>
void CompanEdgeBoostServerConnection<T>::parseRequestMessage(char const* data, size_t const len)
{
    CompanEdgeProtocol::ServerMessage msg;
    if (!msg.ParseFromArray(data, len)) {
        ErrorLog(AebServerConnectionLog) << "[" << connectionId_ << "] onPayload, parse error - closing connection"
                                         << std::endl;
        return doClose();
    }

    size_t queueSize = 0;

    {
        std::lock_guard<std::mutex> lock(serverMutex_);
        serverQueue_.push(msg);

        queueSize = serverQueue_.size();
    }

    if (queueSize == 1)
        boost::asio::post(
                ioContext_, std::bind(&CompanEdgeBoostServerConnection::doServerQueue, this->shared_from_this()));
}

template <typename T>
void CompanEdgeBoostServerConnection<T>::doServerQueue()
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "]" << std::endl;

    CompanEdgeProtocol::ServerMessage msg;

    size_t queueSize = 0;

    // We hold onto the lock for this whole process
    // - or the messageHandlerPtr_ might go out of scope
    //  Perhaps we should "post" to it?

    std::lock_guard<std::mutex> lock(serverMutex_);

    if (serverQueue_.empty()) { return; }

    msg = serverQueue_.front();
    serverQueue_.pop();

    queueSize = serverQueue_.size();

    if (!messageHandlerPtr_) {
        ErrorLog(AebServerConnectionLog) << "[" << connectionId_ << "]"
                                         << " Message Handler disappeared?" << std::endl;

        return;
    }

    messageHandlerPtr_->handleMessage(msg);

    if (queueSize)
        boost::asio::post(
                ioContext_, std::bind(&CompanEdgeBoostServerConnection::doServerQueue, this->shared_from_this()));
}

template <typename T>
void CompanEdgeBoostServerConnection<T>::doClose()
{
    FunctionArgLog(AebServerConnectionLog) << "[" << connectionId_ << "]" << std::endl;

    onDisconnectSignal_(this->shared_from_this());
}

template <typename T>
uint32_t CompanEdgeBoostServerConnection<T>::getConnectionId()
{
    return connectionId_;
}

template <typename T>
typename CompanEdgeBoostServerConnection<T>::EndPointType CompanEdgeBoostServerConnection<T>::endpoint()
{
    return socket_.remote_endpoint();
}

template <typename T>
SignalConnection CompanEdgeBoostServerConnection<T>::connectDisconnectListener(
        typename DisconnectSignal::SlotType const& cb)
{
    return onDisconnectSignal_.connect(cb);
}

template class COMPAN::REF::CompanEdgeBoostServerConnection<boost::asio::ip::tcp>;
template class COMPAN::REF::CompanEdgeBoostServerConnection<boost::asio::local::stream_protocol>;

} // namespace Edge
} // namespace Compan
