/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_tcp_server.cpp
  @brief CompanEdge Boost - TCP server
*/

#include "company_ref_boost_tcp_server.h"

#include <Compan_logger/Compan_logger.h>

#include <boost/asio/ip/tcp.hpp>

namespace Compan{
namespace Edge {
CompanLogger AebTcpServerLog("server.connection.tcp", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

uint32_t CompanEdgeBoostTcpServer::connectionId_(100);

CompanEdgeBoostTcpServer::CompanEdgeBoostTcpServer(
        boost::asio::io_context& ioContext,
        CompanEdgeBoostMessageHandlerFactory& handlerFactory,
        std::string const& address,
        std::string const& port)
    : CompanEdgeBoostServer<boost::asio::ip::tcp>(ioContext, handlerFactory)
{
    boost::asio::ip::tcp::resolver resolver(ioContext);
    boost::asio::ip::tcp::resolver::query query(address, port);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

    doAccept(endpoint);
}

CompanEdgeBoostTcpServer::~CompanEdgeBoostTcpServer() = default;

void CompanEdgeBoostTcpServer::createConnection(SocketType&& socket)
{
    connectionManager_.start(std::make_shared<CompanEdgeBoostServerConnection<boost::asio::ip::tcp>>(
            connectionId_++, ioContext_, std::move(socket), handlerFactory_));
}
