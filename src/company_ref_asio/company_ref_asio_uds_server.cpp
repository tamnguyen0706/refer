/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_uds_server.cpp
 @brief Derived Server for UDS sockets
 */
#include "company_ref_asio_uds_server.h"
#include "company_ref_asio_uds_connection.h"

#include "company_ref_asio_msg_handler.h"
#include "company_ref_asio_msg_handler_factory.h"

using namespace Compan::Edge;

uint32_t AsioUdsServer::connectionId_(1000);

AsioUdsServer::AsioUdsServer(boost::asio::io_context& ctx, AsioMsgHandlerFactory& factory, std::string const& udsPath)
    : AsioServer<boost::asio::local::stream_protocol>(ctx, factory)
{
    if (!udsPath.empty()) {
        ::unlink(udsPath.c_str()); // Remove previous binding.
    }

    boost::asio::local::stream_protocol::endpoint endpoint(udsPath);

    doAccept(endpoint);
}

AsioUdsServer::~AsioUdsServer() = default;

void AsioUdsServer::createConnection(SocketType&& socket)
{
    ++connectionId_;

    AsioMsgHandlerPtr msgHandler = factory_.make(connectionId_);

    AsioUdsConnection::Ptr connPtr(
            std::make_shared<AsioUdsConnection>(ctx_, connectionId_, msgHandler, std::move(socket)));

    {
        std::lock_guard<std::mutex> lock(mutex_);

        SignalScopedConnection disconnectSignal(connPtr->connectOnDisconnection(
                std::bind(&AsioUdsServer::onDisconnection, this, std::placeholders::_1)));

        connections_.emplace(connectionId_, connPtr);
        signalConnections_.emplace(connectionId_, std::move(disconnectSignal));
    }

    connPtr->onConnect(boost::system::error_code());
}
