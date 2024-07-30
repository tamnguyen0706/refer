/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_server.cpp
  @brief CompanEdge Boost - server
*/

#include "company_ref_boost_server.h"

#include <Compan_logger/Compan_logger.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>

namespace Compan{
namespace Edge {
CompanLogger CompanEdgeBoostServerBaseLog("server.listener.core", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

CompanEdgeBoostServerBase::CompanEdgeBoostServerBase(boost::asio::io_context& ioContext)
    : ioContext_(ioContext)

{
}

CompanEdgeBoostServerBase::~CompanEdgeBoostServerBase() = default;

template <typename T>
CompanEdgeBoostServer<T>::CompanEdgeBoostServer(
        boost::asio::io_context& ioContext,
        CompanEdgeBoostMessageHandlerFactory& handlerFactory)
    : CompanEdgeBoostServerBase(ioContext)
    , socket_(ioContext)
    , acceptor_(ioContext)
    , handlerFactory_(handlerFactory)
    , connectionManager_(ioContext)
{
    FunctionLog(CompanEdgeBoostServerBaseLog);
}

template <typename T>
CompanEdgeBoostServer<T>::~CompanEdgeBoostServer() = default;

template <typename T>
void CompanEdgeBoostServer<T>::stop()
{
    FunctionLog(CompanEdgeBoostServerBaseLog);

    if (!acceptor_.is_open()) return;

    acceptor_.close();
    connectionManager_.stopAll();
}

template <typename T>
typename CompanEdgeBoostServer<T>::ConnectionManagerType& CompanEdgeBoostServer<T>::connectionManager()
{
    return connectionManager_;
}

template <typename T>
void CompanEdgeBoostServer<T>::doAccept(typename SocketType::endpoint_type const& endpoint)
{
    FunctionLog(CompanEdgeBoostServerBaseLog);

    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(typename T::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    doAccept();
}

template <typename T>
void CompanEdgeBoostServer<T>::doAccept()
{
    acceptor_.async_accept([this](boost::system::error_code ec, SocketType socket) {
        if (ec) {
            DebugLog(CompanEdgeBoostServerBaseLog) << ec.message() << std::endl;
            return;
        }

        createConnection(std::move(socket));
        doAccept();
    });
}

template class COMPAN::REF::CompanEdgeBoostServer<boost::asio::ip::tcp>;
template class COMPAN::REF::CompanEdgeBoostServer<boost::asio::local::stream_protocol>;
