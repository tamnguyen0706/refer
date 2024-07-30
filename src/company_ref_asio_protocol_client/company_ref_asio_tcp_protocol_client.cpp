/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_tcp_protocol_client.cpp
 @brief Client class for TCP sockets
 */
#include "company_ref_asio_tcp_protocol_client.h"
#include "company_ref_asio_client_protocol_serializer.h"

using namespace Compan::Edge;

AsioTcpProtocolClient::AsioTcpProtocolClient(
        boost::asio::io_context& ctx,
        ClientProtocolHandlerPtr clientProtocolHandler,
        std::string const& host,
        std::string const& port)
    : AsioTcpConnection(ctx, 0, std::make_shared<ClientProtocolSerializer>(ctx, clientProtocolHandler, 0), host, port)
{
}
