/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_tcp_protocol_client.h
 @brief Client class for TCP sockets
 */
#ifndef __company_ref_ASIO_TCP_PROTOCOL_CLIENT_H__
#define __company_ref_ASIO_TCP_PROTOCOL_CLIENT_H__

#include <company_ref_asio/company_ref_asio_tcp_connection.h>

namespace Compan{
namespace Edge {

class ClientProtocolHandler;
using ClientProtocolHandlerPtr = std::shared_ptr<ClientProtocolHandler>;

/*!
 *
 */
class AsioTcpProtocolClient : public AsioTcpConnection {
public:
    AsioTcpProtocolClient(
            boost::asio::io_context&,
            ClientProtocolHandlerPtr clientProtocolHandler,
            std::string const&,
            std::string const&);

    virtual ~AsioTcpProtocolClient() = default;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_TCP_PROTOCOL_CLIENT_H__
