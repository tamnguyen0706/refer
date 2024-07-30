/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_uds_connection.h
 @brief Derived Connection class for UDS sockets
 */
#ifndef __company_ref_ASIO_UDS_CONNECTION_H__
#define __company_ref_ASIO_UDS_CONNECTION_H__

#include "company_ref_asio_connection.h"

#include <boost/asio/local/stream_protocol.hpp>

namespace Compan{
namespace Edge {

class AsioUdsConnection : public AsioConnection<boost::asio::local::stream_protocol> {
public:
    using Ptr = std::shared_ptr<AsioUdsConnection>;

    using BaseClass = AsioConnection<boost::asio::local::stream_protocol>;

    /// Server UDS connection
    AsioUdsConnection(
            boost::asio::io_context& ctx,
            uint32_t const connId,
            AsioMsgHandlerPtr msgHandler,
            BaseClass::SocketType socket);

    /// Client UDS connection
    AsioUdsConnection(
            boost::asio::io_context& ctx,
            uint32_t const connId,
            AsioMsgHandlerPtr msgHandler,
            std::string const& udsPath);

    virtual ~AsioUdsConnection();
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_UDS_CONNECTION_H__
