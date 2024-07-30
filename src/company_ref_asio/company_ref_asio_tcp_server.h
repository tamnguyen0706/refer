/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_tcp_server.h
 @brief Derived Server for TCP sockets
 */
#ifndef __company_ref_ASIO_TCP_SERVER_H__
#define __company_ref_ASIO_TCP_SERVER_H__

#include "company_ref_asio_server.h"

#include <boost/asio/ip/tcp.hpp>

namespace Compan{
namespace Edge {

class AsioTcpServer : public AsioServer<boost::asio::ip::tcp> {
public:
    AsioTcpServer(
            boost::asio::io_context& ctx,
            AsioMsgHandlerFactory& factory,
            std::string const& address,
            std::string const& port);
    virtual ~AsioTcpServer();

protected:
    virtual void createConnection(SocketType&& socket);

private:
    static uint32_t connectionId_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_TCP_SERVER_H__
