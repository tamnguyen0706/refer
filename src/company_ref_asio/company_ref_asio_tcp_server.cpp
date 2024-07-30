/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_tcp_server.cpp
 @brief Derived Server for TCP sockets
 */
#include "company_ref_asio_tcp_server.h"
#include "company_ref_asio_tcp_connection.h"

#include "company_ref_asio_msg_handler.h"
#include "company_ref_asio_msg_handler_factory.h"

using namespace Compan::Edge;

uint32_t AsioTcpServer::connectionId_(100);

AsioTcpServer::AsioTcpServer(
        boost::asio::io_context& ctx,
        AsioMsgHandlerFactory& factory,
        std::string const& address,
        std::string const& port)
    : AsioServer<boost::asio::ip::tcp>(ctx, factory)
{
    boost::asio::ip::tcp::resolver resolver(ctx);
    boost::asio::ip::tcp::resolver::query query(address, port);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

    doAccept(endpoint);
}

AsioTcpServer::~AsioTcpServer() = default;

void AsioTcpServer::createConnection(SocketType&& socket)
{
    ++connectionId_;

    AsioMsgHandlerPtr msgHandler = factory_.make(connectionId_);
    AsioTcpConnection::Ptr connPtr(
            std::make_shared<AsioTcpConnection>(ctx_, connectionId_, msgHandler, std::move(socket)));

    {
        std::lock_guard<std::mutex> lock(mutex_);

        SignalScopedConnection disconnectSignal(connPtr->connectOnDisconnection(
                std::bind(&AsioTcpServer::onDisconnection, this, std::placeholders::_1)));

        connections_.emplace(connectionId_, connPtr);
        signalConnections_.emplace(connectionId_, std::move(disconnectSignal));
    }

    connPtr->onConnect(boost::system::error_code());
}
