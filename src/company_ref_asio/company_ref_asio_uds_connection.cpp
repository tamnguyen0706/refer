/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_uds_connection.cpp
 @brief Derived Connection class for UDS sockets
 */
#include "company_ref_asio_uds_connection.h"

#include "company_ref_asio_msg_handler.h"

using namespace Compan::Edge;

AsioUdsConnection::AsioUdsConnection(
        boost::asio::io_context& ctx,
        uint32_t const connId,
        AsioMsgHandlerPtr msgHandler,
        BaseClass::SocketType socket)
    : AsioConnection<boost::asio::local::stream_protocol>(ctx, connId, msgHandler, std::move(socket))
{
}

AsioUdsConnection::AsioUdsConnection(
        boost::asio::io_context& ctx,
        uint32_t const connId,
        AsioMsgHandlerPtr msgHandler,
        std::string const& udsPath)
    : AsioConnection<boost::asio::local::stream_protocol>(ctx, connId, msgHandler)
{
    endPoint_ = EndPointType(udsPath);
}

AsioUdsConnection::~AsioUdsConnection() = default;
