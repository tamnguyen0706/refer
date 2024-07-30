/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_tcp_client.cpp
  @brief CompanEdge Boost - TCP client
*/

#include "company_ref_boost_tcp_client.h"

#include <Compan_logger/Compan_logger.h>

namespace Compan{
namespace Edge {
CompanLogger AebTcpClientLog("client.connection.tcp", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

CompanEdgeBoostTcpClient::CompanEdgeBoostTcpClient(
        boost::asio::io_context& ioContext,
        std::string const& host,
        std::string const& service)
    : CompanEdgeBoostClient<boost::asio::ip::tcp>(ioContext)
    , resolver_(ioContext)
{
    boost::asio::ip::tcp::resolver::query query(host, service);
    doResolve(query);
}

CompanEdgeBoostTcpClient::~CompanEdgeBoostTcpClient() = default;

void CompanEdgeBoostTcpClient::doResolve(boost::asio::ip::tcp::resolver::query const& query)
{
    FunctionLog(AebTcpClientLog);
    // this is coming from the constructor - shared_from_this() isn't valid yet
    resolver_.async_resolve(
            query, std::bind(&CompanEdgeBoostTcpClient::onResolve, this, std::placeholders::_1, std::placeholders::_2));
}

void CompanEdgeBoostTcpClient::onResolve(
        boost::system::error_code const& ec,
        boost::asio::ip::tcp::resolver::iterator iterator)
{
    FunctionLog(AebTcpClientLog);

    if (ec) {
        ErrorLog(AebTcpClientLog) << "onResolve, error: " << ec.message() << std::endl;
        return;
    }
    doResolveConnect(iterator);
}

void CompanEdgeBoostTcpClient::doResolveConnect(boost::asio::ip::tcp::resolver::iterator iterator)
{
    FunctionLog(AebTcpClientLog);

    endPoint_ = *iterator;
    socket_.async_connect(
            endPoint_, std::bind(&CompanEdgeBoostTcpClient::onResolveConnect, this, std::placeholders::_1, ++iterator));
}

void CompanEdgeBoostTcpClient::onResolveConnect(
        boost::system::error_code const& ec,
        boost::asio::ip::tcp::resolver::iterator iterator)
{
    FunctionLog(AebTcpClientLog);

    if (!ec) {
        // now that we have a REAL endpoint, call the super class's onConnect
        onConnect(ec);
    } else if (iterator != boost::asio::ip::tcp::resolver::iterator()) {
        doClose();
        return onResolve(boost::system::error_code(), iterator);
    } else {
        ErrorLog(AebTcpClientLog) << "onConnect, error: " << ec.message() << std::endl;
    }
}
