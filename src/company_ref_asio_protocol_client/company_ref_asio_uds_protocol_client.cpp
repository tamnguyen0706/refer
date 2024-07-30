/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_uds_protocol_client.cpp
 @brief Client class for UDS sockets
 */
#include "company_ref_asio_uds_protocol_client.h"
#include "company_ref_asio_client_protocol_serializer.h"

using namespace Compan::Edge;

AsioUdsProtocolClient::AsioUdsProtocolClient(
        boost::asio::io_context& ctx,
        ClientProtocolHandlerPtr clientProtocolHandler,
        std::string const& udsPath)
    : AsioUdsConnection(ctx, 0, std::make_shared<ClientProtocolSerializer>(ctx, clientProtocolHandler, 0), udsPath)
{
}
