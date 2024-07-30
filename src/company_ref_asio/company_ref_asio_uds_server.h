/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_uds_server.h
 @brief Derived Server for UDS sockets
 */
#ifndef __company_ref_ASIO_UDS_SERVER_H__
#define __company_ref_ASIO_UDS_SERVER_H__

#include "company_ref_asio_server.h"

#include <boost/asio/local/stream_protocol.hpp>

namespace Compan{
namespace Edge {

class AsioUdsServer : public AsioServer<boost::asio::local::stream_protocol> {
public:
    AsioUdsServer(boost::asio::io_context& ctx, AsioMsgHandlerFactory& factory, std::string const& udsPath);
    virtual ~AsioUdsServer();

protected:
    virtual void createConnection(SocketType&& socket);

private:
    static uint32_t connectionId_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_UDS_SERVER_H__
