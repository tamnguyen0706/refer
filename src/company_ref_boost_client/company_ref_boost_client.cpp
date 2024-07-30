/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_client.cpp
  @brief CompanEdge Boost - client
*/

#include "company_ref_boost_client.h"

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_framing.h>
#include <company_ref_protocol_utils/protobuf.h>
#include <company_ref_utils/company_ref_weak_bind.h>

#include <Compan_logger/Compan_logger.h>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

namespace Compan{
namespace Edge {
CompanLogger AebClientLog("client.connection.core", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

CompanEdgeBoostClientBase::CompanEdgeBoostClientBase(boost::asio::io_context& ioContext)
    : ioContext_(ioContext)
    , readerStrand_(ioContext_)
    , fnWantRead_([]() { return false; })
    , onClientConnectedSignal_(ioContext)
    , onClientDisconnectedSignal_(ioContext)
    , onClientMessageSignal_(ioContext)
    , bNewFraming_(true)
    , pAecCallbacks_(new AecCallbacks<std::string>(
              {[this]() {
                   DebugLog(AebClientLog) << "Parse error, closing connection..." << std::endl;
                   doClose();
               },
               [this](AECFrameId const& frameId, std::string::const_iterator begin, std::string::const_iterator end) {
                   DebugLog(AebClientLog) << "Received AEC frame with id " << frameId.str() << " and size "
                                          << std::distance(begin, end) << std::endl;
                   if ((frameId == AECv09::frameId) || (frameId == AECv10::frameId)) {
                       parseResponseMessage(&(*begin), std::distance(begin, end));
                   } else if (frameId.str() == AECv10::cUnknownFrameId) {
                       ErrorLog(AebClientLog) << "Received unknown frameId (" << frameId.str() << ')' << std::endl;
                   }
               }}))
{
    Protobuf::instance();
}

CompanEdgeBoostClientBase::~CompanEdgeBoostClientBase() = default;

void CompanEdgeBoostClientBase::connect()
{
    FunctionLog(AebClientLog);

    /// Do clear readBuffer_ here because the application will be crashed if doing it at doClose.
    readBuffer_.erase();
    bNewFraming_ = true;
    doConnect();
}

void CompanEdgeBoostClientBase::close()
{
    doClose();
}

void CompanEdgeBoostClientBase::setWantRead(fnWantRead wantRead)
{
    FunctionLog(AebClientLog);

    fnWantRead_ = wantRead;
}

void CompanEdgeBoostClientBase::write(CompanEdgeProtocol::ServerMessage const& msg)
{
    FunctionLog(AebClientLog);

    boost::asio::post(ioContext_, [this, msg]() {
        size_t queueSize = 0;

        {
            std::lock_guard<std::mutex> lock(serverMutex_);
            serverQueue_.push(msg);

            queueSize = serverQueue_.size();
        }

        if (queueSize == 1) asyncDoWrite();
    });
}

void CompanEdgeBoostClientBase::asyncDoWrite()
{
    boost::asio::post(ioContext_, std::bind(&CompanEdgeBoostClientBase::doWrite, this->shared_from_this()));
}

void CompanEdgeBoostClientBase::onConnect(boost::system::error_code const& ec)
{
    FunctionLog(AebClientLog);

    if (ec) {
        DebugLog(AebClientLog) << "onConnect, error: " << ec.message() << std::endl;
        return;
    }

    onClientConnectedSignal_();
    if (fnWantRead_()) doRead();
}

void CompanEdgeBoostClientBase::onRead(boost::system::error_code const& ec, std::size_t length)
{
    FunctionArgLog(AebClientLog) << "len=" << length << ", ec=" << ec << std::endl;

    if (ec) {
        DebugLog(AebClientLog) << "onRead, Closing connection" << std::endl;
        return doClose();
    }

    if (length == 0) {
        ErrorLog(AebClientLog) << "onRead, parse error - closing connection, len:" << length << std::endl;
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
    if (fnWantRead_()) doRead();
}

void CompanEdgeBoostClientBase::parseResponseMessage(char const* data, size_t const len)
{
    CompanEdgeProtocol::ClientMessage msg;
    if (!msg.ParseFromArray(data, len)) {
        ErrorLog(AebClientLog) << "onRead, parse error - closing connection" << std::endl;
        return doClose();
    }

    size_t queueSize = 0;

    {
        std::lock_guard<std::mutex> lock(clientMutex_);
        clientQueue_.push(msg);

        queueSize = clientQueue_.size();
    }

    if (queueSize == 1)
        boost::asio::post(ioContext_, std::bind(&CompanEdgeBoostClientBase::doClientQueue, this->shared_from_this()));
}

void CompanEdgeBoostClientBase::doClientQueue()
{
    CompanEdgeProtocol::ClientMessage msg;

    {
        std::lock_guard<std::mutex> lock(clientMutex_);

        if (clientQueue_.empty()) { return; }

        msg = clientQueue_.front();
        clientQueue_.pop();
    }

    onClientMessageSignal_(msg);

    /// want read needs to get deprecated, in favor of just calling a cancellation
    if (!fnWantRead_()) {
        doCancel();
        return;
    }

    boost::asio::post(ioContext_, std::bind(&CompanEdgeBoostClientBase::doClientQueue, this->shared_from_this()));
}

template <typename T>
CompanEdgeBoostClient<T>::CompanEdgeBoostClient(boost::asio::io_context& ioContext)
    : CompanEdgeBoostClientBase(ioContext)
    , socket_(ioContext)
    , endPoint_()
{
}

template <typename T>
CompanEdgeBoostClient<T>::~CompanEdgeBoostClient() = default;

template <typename T>
bool CompanEdgeBoostClient<T>::isConnected()
{
    return socket_.is_open();
}

template <typename T>
void CompanEdgeBoostClient<T>::doCancel()
{
    if (socket_.is_open()) socket_.cancel();
}

template <typename T>
void CompanEdgeBoostClient<T>::doClose()
{
    onClientDisconnectedSignal_();
    boost::system::error_code ec;
    socket_.close(ec);
    if (ec) {
        ErrorLog(AebClientLog) << __FUNCTION__ << ec.message() << std::endl;
        return;
    }

    {
        std::lock_guard<std::mutex> lock(serverMutex_);
        while (!serverQueue_.empty()) serverQueue_.pop();
    }

    {
        std::lock_guard<std::mutex> lock(clientMutex_);
        while (!clientQueue_.empty()) clientQueue_.pop();
    }
}

template <typename T>
void CompanEdgeBoostClient<T>::doClientConnected()
{
    FunctionLog(AebClientLog);

    onClientConnectedSignal_();
    if (fnWantRead_()) doRead();
}

template <typename T>
void CompanEdgeBoostClient<T>::doRead()
{
    FunctionLog(AebClientLog);

    boost::asio::async_read(
            socket_,
            boost::asio::dynamic_buffer(readBuffer_),
            boost::asio::transfer_at_least(1),
            boost::asio::bind_executor(
                    readerStrand_,
                    WeakBind(
                            &CompanEdgeBoostClient<T>::onRead,
                            this->shared_from_this(),
                            std::placeholders::_1,
                            std::placeholders::_2)));
}

template <typename T>
void CompanEdgeBoostClient<T>::doWrite()
{
    FunctionLog(AebClientLog);

    if (!socket_.is_open()) return doClose();

    CompanEdgeProtocol::ServerMessage msg;

    size_t queueSize = 0;

    {
        std::lock_guard<std::mutex> lock(serverMutex_);

        if (serverQueue_.empty()) { return; }

        msg = serverQueue_.front();
        serverQueue_.pop();

        queueSize = serverQueue_.size();
    }

    std::string reqBuffer;
    msg.SerializeToString(&reqBuffer);

    if (bNewFraming_) {
        reqBuffer.insert(0, AECv10::makeHeader<std::string>(reqBuffer.cbegin(), reqBuffer.cend()));
    } else {
        reqBuffer.insert(0, AECv09::makeHeader<std::string>(reqBuffer.cbegin(), reqBuffer.cend()));
    }

    boost::system::error_code ec;

    boost::asio::write(socket_, boost::asio::buffer(reqBuffer.data(), reqBuffer.size()), ec);
    if (ec) {
        DebugLog(AebClientLog) << " Closing connection" << std::endl;
        return doClose();
    }

    if (queueSize) asyncDoWrite();
}

template <typename T>
void CompanEdgeBoostClient<T>::doConnect()
{
    FunctionLog(AebClientLog);

    socket_.async_connect(
            endPoint_, std::bind(&CompanEdgeBoostClient<T>::onConnect, this->shared_from_this(), std::placeholders::_1));
}

template class COMPAN::REF::CompanEdgeBoostClient<boost::asio::ip::tcp>;
template class COMPAN::REF::CompanEdgeBoostClient<boost::asio::local::stream_protocol>;
