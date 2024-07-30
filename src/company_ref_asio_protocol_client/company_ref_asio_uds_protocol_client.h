/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_uds_protocol_client.h
 @brief Client class for UDS sockets
 */
#ifndef __company_ref_ASIO_UDS_PROTOCOL_CLIENT_H__
#define __company_ref_ASIO_UDS_PROTOCOL_CLIENT_H__

#include <company_ref_asio/company_ref_asio_uds_connection.h>

namespace Compan{
namespace Edge {

class ClientProtocolHandler;
using ClientProtocolHandlerPtr = std::shared_ptr<ClientProtocolHandler>;

/*!
 *
 */
class AsioUdsProtocolClient : public AsioUdsConnection {
public:
    AsioUdsProtocolClient(boost::asio::io_context&, ClientProtocolHandlerPtr clientProtocolHandler, std::string const&);

    virtual ~AsioUdsProtocolClient() = default;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_UDS_PROTOCOL_CLIENT_H__
