/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_tcp_server.h
  @brief CompanEdge Boost - TCP server
*/

#ifndef __company_ref_BOOST_TCP_SERVER_H__
#define __company_ref_BOOST_TCP_SERVER_H__

#include "company_ref_boost_handler_factory.h"
#include "company_ref_boost_server.h"
#include "company_ref_boost_server_connection_manager.h"

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

#include <boost/asio/ip/tcp.hpp>

namespace Compan{
namespace Edge {

class CompanEdgeBoostTcpServer : public CompanEdgeBoostServer<boost::asio::ip::tcp> {
public:
    CompanEdgeBoostTcpServer(
            boost::asio::io_context&,
            CompanEdgeBoostMessageHandlerFactory& handlerFactory,
            std::string const& address,
            std::string const& port);
    virtual ~CompanEdgeBoostTcpServer();

protected:
    virtual void createConnection(SocketType&& socket);

private:
    static uint32_t connectionId_;
};

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_BOOST_TCP_SERVER_H__*/
