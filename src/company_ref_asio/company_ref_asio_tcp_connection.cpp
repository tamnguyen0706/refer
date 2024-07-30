/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_tcp_connection.cpp
 @brief Derived Connection class for TCP sockets
 */
#include "company_ref_asio_tcp_connection.h"

#include "company_ref_asio_msg_handler.h"
#include <company_ref_utils/company_ref_weak_bind.h>

#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

CompanLogger AsioTcpConnectionLog("asio.connection.tcp", LogLevel::Information);

AsioTcpConnection::AsioTcpConnection(
        boost::asio::io_context& ctx,
        uint32_t const connId,
        AsioMsgHandlerPtr msgHandler,
        BaseClass::SocketType socket)
    : AsioConnection<boost::asio::ip::tcp>(ctx, connId, msgHandler, std::move(socket))
    , resolver_(ctx)
{
}

AsioTcpConnection::AsioTcpConnection(
        boost::asio::io_context& ctx,
        uint32_t const connId,
        AsioMsgHandlerPtr msgHandler,
        std::string const& host,
        std::string const& port)
    : AsioConnection<boost::asio::ip::tcp>(ctx, connId, msgHandler)
    , resolver_(ctx)
    , host_(host)
    , port_(port)
{
}

AsioTcpConnection::~AsioTcpConnection() = default;

void AsioTcpConnection::connect()
{
    boost::asio::ip::tcp::resolver::query query(host_, port_);
    doResolve(query);
}

void AsioTcpConnection::doResolve(boost::asio::ip::tcp::resolver::query const& query)
{
    FunctionArgLog(AsioTcpConnectionLog) << loggerIdentity() << std::endl;

    resolver_.async_resolve(
            query,
            WeakBind(&AsioTcpConnection::onResolve, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsioTcpConnection::onResolve(
        boost::system::error_code const& ec,
        boost::asio::ip::tcp::resolver::iterator iterator)
{
    FunctionArgLog(AsioTcpConnectionLog) << loggerIdentity() << std::endl;

    if (ec) {
        ErrorLog(AsioTcpConnectionLog) << loggerIdentity() << " onResolve: " << ec.message() << std::endl;
        return;
    }
    doResolveConnect(iterator);
}

void AsioTcpConnection::doResolveConnect(boost::asio::ip::tcp::resolver::iterator iterator)
{
    FunctionArgLog(AsioTcpConnectionLog) << loggerIdentity() << std::endl;

    endPoint_ = *iterator;
    socket_.async_connect(
            endPoint_,
            WeakBind(&AsioTcpConnection::onResolveConnect, shared_from_this(), std::placeholders::_1, ++iterator));
}

void AsioTcpConnection::onResolveConnect(
        boost::system::error_code const& ec,
        boost::asio::ip::tcp::resolver::iterator iterator)
{
    FunctionArgLog(AsioTcpConnectionLog) << loggerIdentity() << std::endl;

    if (!ec) {
        // now that we have a REAL endpoint, call the super class's onConnect
        onConnect(ec);
    } else if (iterator != boost::asio::ip::tcp::resolver::iterator()) {
        doClose();
        return onResolve(boost::system::error_code(), iterator);
    } else {
        ErrorLog(AsioTcpConnectionLog) << loggerIdentity() << " onConnect: " << ec.message() << std::endl;
    }
}
