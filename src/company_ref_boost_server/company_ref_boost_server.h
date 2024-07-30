/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_server.h
  @brief CompanEdge Boost - server
*/

#ifndef __company_ref_BOOST_SERVER_H__
#define __company_ref_BOOST_SERVER_H__

#include "company_ref_boost_handler_factory.h"
#include "company_ref_boost_server_connection_manager.h"

#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

namespace Compan{
namespace Edge {

class CompanEdgeBoostServerBase {
public:
    CompanEdgeBoostServerBase(boost::asio::io_context&);
    virtual ~CompanEdgeBoostServerBase();

protected:
    // The io_context used to perform asynchronous operations.
    boost::asio::io_context& ioContext_;
};

template <typename T>
class CompanEdgeBoostServer : public CompanEdgeBoostServerBase {
public:
    using SocketType = boost::asio::basic_stream_socket<T>;
    using AcceptorType = boost::asio::basic_socket_acceptor<T>;
    using ConnectionManagerType = CompanEdgeBoostServerConnectionManager<T>;
    using ConnectionPtr = typename ConnectionManagerType::BoostServerConnectionPtr;

    CompanEdgeBoostServer(boost::asio::io_context&, CompanEdgeBoostMessageHandlerFactory& handlerFactory);
    virtual ~CompanEdgeBoostServer();

    ConnectionManagerType& connectionManager();

    void stop();

protected:
    void doAccept(typename SocketType::endpoint_type const& endpoint);
    void doAccept();

    virtual void createConnection(SocketType&& socket) = 0;

protected:
    SocketType socket_;
    AcceptorType acceptor_;

    CompanEdgeBoostMessageHandlerFactory& handlerFactory_;

    // The connection manager which owns all live connections.
    CompanEdgeBoostServerConnectionManager<T> connectionManager_;
};

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_BOOST_SERVER_H__*/
