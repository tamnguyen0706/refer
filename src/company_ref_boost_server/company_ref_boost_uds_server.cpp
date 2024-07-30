/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_uds_server.cpp
  @brief CompanEdge Boost - UDS server
*/

#include "company_ref_boost_uds_server.h"

#include <Compan_logger/Compan_logger.h>

#include <boost/asio/local/stream_protocol.hpp>

namespace Compan{
namespace Edge {
CompanLogger AebUdsServerLog("server.connection.uds", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

uint32_t CompanEdgeBoostUdsServer::connectionId_(1000);

CompanEdgeBoostUdsServer::CompanEdgeBoostUdsServer(
        boost::asio::io_context& ioContext,
        CompanEdgeBoostMessageHandlerFactory& handlerFactory,
        std::string const& path)
    : CompanEdgeBoostServer<boost::asio::local::stream_protocol>(ioContext, handlerFactory)
{
    if (!path.empty()) {
        ::unlink(path.c_str()); // Remove previous binding.
    }
    boost::asio::local::stream_protocol::endpoint endpoint(path);

    doAccept(endpoint);
}

CompanEdgeBoostUdsServer::~CompanEdgeBoostUdsServer() = default;

void CompanEdgeBoostUdsServer::createConnection(SocketType&& socket)
{
    connectionManager_.start(std::make_shared<CompanEdgeBoostServerConnection<boost::asio::local::stream_protocol>>(
            connectionId_++, ioContext_, std::move(socket), handlerFactory_));
}
