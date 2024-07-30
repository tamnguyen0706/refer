/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_uds_server.h
  @brief CompanEdge Boost - UDS server
*/

#ifndef __company_ref_BOOST_UDS_SERVER_H__
#define __company_ref_BOOST_UDS_SERVER_H__

#include "company_ref_boost_server.h"
#include "company_ref_boost_server_connection_manager.h"

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

#include <boost/asio/local/stream_protocol.hpp>

namespace Compan{
namespace Edge {

class CompanEdgeBoostUdsServer : public CompanEdgeBoostServer<boost::asio::local::stream_protocol> {
public:
    CompanEdgeBoostUdsServer(
            boost::asio::io_context&,
            CompanEdgeBoostMessageHandlerFactory& handlerFactory,
            std::string const& udsPath);
    virtual ~CompanEdgeBoostUdsServer();

protected:
    virtual void createConnection(SocketType&& socket);

private:
    static uint32_t connectionId_;
};

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_BOOST_UDS_SERVER_H__*/
