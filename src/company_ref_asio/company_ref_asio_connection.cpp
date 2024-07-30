/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_connection.cpp
 @brief Socket type Agnostic Connection class
 */
#include "company_ref_asio_connection.h"

#include "company_ref_asio_msg_handler.h"

#include <company_ref_utils/company_ref_weak_bind.h>
#include <Compan_logger/Compan_logger.h>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

using namespace Compan::Edge;

CompanLogger AsioConnectionLog("asio.connection", LogLevel::Information);

std::vector<std::string> const AsioConnectionBase::ConnectionTypeStr({"Client", "Server"});

AsioConnectionBase::AsioConnectionBase(
        boost::asio::io_context& ctx,
        uint32_t const connectionId,
        ConnectionType const connectionType)
    : readerStrand_(ctx)
    , connectionId_(connectionId)
    , connectionType_(connectionType)
    , onConnected_(ctx)
    , onDisconnected_(ctx)
{
}

std::string AsioConnectionBase::loggerIdentity()
{
    std::stringstream strm;

    strm << " " << ConnectionTypeStr[connectionType_] << "[" << connectionId_ << "]";

    return strm.str();
}

template <typename T>
AsioConnection<T>::AsioConnection(
        boost::asio::io_context& ctx,
        uint32_t const connectionId,
        AsioMsgHandler::Ptr msgHandler)
    : AsioConnectionBase(ctx, connectionId, AsioConnectionBase::Client)
    , socket_(ctx)
    , dataHandler_(msgHandler)
    , doClose_(false)
{
    FunctionArgLog(AsioConnectionLog) << "ctor 1 - " << connectionId_ << std::endl;

    if (dataHandler_ == nullptr) {
        ErrorLog(AsioConnectionLog) << "missing MsgHandler - " << connectionId_ << std::endl;
    }
}

template <typename T>
AsioConnection<T>::AsioConnection(
        boost::asio::io_context& ctx,
        uint32_t const connId,
        AsioMsgHandler::Ptr msgHandler,
        SocketType socket)
    : AsioConnection(ctx, connId, msgHandler)
{
    FunctionArgLog(AsioConnectionLog) << "ctor 2 - " << connectionId_ << std::endl;

    // delegating ctor's have limitations
    socket_ = std::move(socket);
}

template <typename T>
AsioConnection<T>::~AsioConnection()
{
    FunctionLog(AsioConnectionLog);
}

template <typename T>
void AsioConnection<T>::onConnect(boost::system::error_code const& error)
{
    FunctionArgLog(AsioConnectionLog) << loggerIdentity() << std::endl;

    if (error) {
        ErrorLog(AsioConnectionLog) << loggerIdentity() << " onConnect " << error.message() << std::endl;
        return;
    }

    onConnected_(connectionId_);

    if (!dataHandler_) {
        ErrorLog(AsioConnectionLog) << loggerIdentity() << " onConnect missing MsgHandler" << std::endl;
        return;
    }

    dataHandler_->start();

    onSend_ = dataHandler_->connectSendData(
            WeakBind(&AsioConnection<T>::doSend, this->shared_from_this(), std::placeholders::_1));

    doRead();
}

template <typename T>
void AsioConnection<T>::connect()
{
    FunctionArgLog(AsioConnectionLog) << loggerIdentity() << std::endl;

    doClose_ = false;
    socket_.async_connect(
            endPoint_, WeakBind(&AsioConnection<T>::onConnect, this->shared_from_this(), std::placeholders::_1));
}

template <typename T>
void AsioConnection<T>::close()
{
    FunctionArgLog(AsioConnectionLog) << loggerIdentity() << std::endl;

    // This will invoke the cancellation wrappers for the reads/writes
    if (socket_.is_open()) socket_.cancel();
}

template <typename T>
bool AsioConnection<T>::isConnected()
{
    return socket_.is_open();
}

template <typename T>
typename AsioConnection<T>::EndPointType& AsioConnection<T>::endpoint()
{
    return endPoint_;
}

template <typename T>
void AsioConnection<T>::doClose()
{
    FunctionArgLog(AsioConnectionLog) << __FUNCTION__ << loggerIdentity() << std::endl;

    if (socket_.is_open()) {
        boost::system::error_code ec;
        socket_.close(ec);

        if (ec) { ErrorLog(AsioConnectionLog) << loggerIdentity() << " close " << ec.message() << std::endl; }
    }

    onSend_.reset();

    if (dataHandler_ && !doClose_) { boost::asio::post(readerStrand_, WeakBind(&AsioMsgHandler::stop, dataHandler_)); }

    onDisconnected_(connectionId_);

    doClose_ = true;
}

template <typename T>
void AsioConnection<T>::doRead()
{
    FunctionArgLog(AsioConnectionLog) << loggerIdentity() << std::endl;

    recvBuffer_ = AsioMsgHandler::BufferType();

    // boost::asio::bind_executor with a strand guarantees ordering
    //  of incoming packets
    boost::asio::async_read(
            socket_,
            boost::asio::dynamic_buffer(recvBuffer_),
            boost::asio::transfer_at_least(1),
            boost::asio::bind_executor(
                    readerStrand_,
                    WeakBind(
                            &AsioConnection::onRead,
                            this->shared_from_this(),
                            std::placeholders::_1,
                            std::placeholders::_2)));
}

template <typename T>
void AsioConnection<T>::onRead(boost::system::error_code const& ec, std::size_t bytes_transferred)
{
    FunctionArgLog(AsioConnectionLog) << loggerIdentity() << std::endl;

    if (ec || bytes_transferred == 0) {
        DebugLog(AsioConnectionLog) << loggerIdentity() << " onRead Closing connection" << std::endl;
        doClose();
        return;
    }

    if (!dataHandler_) {
        ErrorLog(AsioConnectionLog) << loggerIdentity() << " onRead Lost data handler?" << std::endl;
        doClose();
        return;
    }

    /// need to change this to a post?
    dataHandler_->recvData(std::move(recvBuffer_));

    doRead();
}

template <typename T>
void AsioConnection<T>::doSend(BufferType&& buffer)
{
    FunctionArgLog(AsioConnectionLog) << loggerIdentity() << std::endl;

    if (!socket_.is_open()) return;

    std::lock_guard<std::mutex> lock(sendLock_);

    boost::system::error_code ec;

    boost::asio::write(socket_, boost::asio::buffer(buffer.data(), buffer.size()), ec);
    if (ec) {

        DebugLog(AsioConnectionLog) << loggerIdentity() << " doSend " << ec.message() << std::endl;

        doClose();
    }
}

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>

template class COMPAN::REF::AsioConnection<boost::asio::ip::tcp>;
template class COMPAN::REF::AsioConnection<boost::asio::local::stream_protocol>;
