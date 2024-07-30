/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_server.h
 @brief Agnostic Socket type connection server
 */
#ifndef __company_ref_ASIO_SERVER_H__
#define __company_ref_ASIO_SERVER_H__

#include <company_ref_utils/company_ref_signals.h>

#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/io_context.hpp>

#include <map>
#include <thread>

namespace Compan{
namespace Edge {

class AsioConnectionBase;
using AsioConnectionPtr = std::shared_ptr<AsioConnectionBase>;

class AsioMsgHandlerFactory;

/*!
 * Handles incoming connections and tracks lifetime of the
 * connection
 */
template <typename T>
class AsioServer {
public:
    using SocketType = boost::asio::basic_stream_socket<T>;
    using AcceptorType = boost::asio::basic_socket_acceptor<T>;
    using BaseType = AsioServer<T>;

    using ConnectionMapType = std::map<uint32_t, AsioConnectionPtr>;

    /// Signal for connection/disconnection
    using ConnectionSignal = SignalAsio<void(uint32_t const)>;

    AsioServer(boost::asio::io_context& ctx, AsioMsgHandlerFactory& factory);
    virtual ~AsioServer();

    /// Closes and disconnects all connections
    void close();

    /// Retrieves a connection
    AsioConnectionPtr get(uint32_t const connectionId);

    /// Iterates the connection list
    typename ConnectionMapType::const_iterator begin();
    typename ConnectionMapType::const_iterator end();

    /// Returns true if no connections exist
    bool empty() const;

protected:
    /// Accepts a connection from an endpoint
    void doAccept(typename SocketType::endpoint_type const& endpoint);

    /// Listens to connections being made
    void doAccept();

    /// Removes a connection id from the server list when a connection
    /// disappears
    void onDisconnection(uint32_t const);

    /// Creates a connection AsioConnection and adds to the server list
    virtual void createConnection(SocketType&& socket) = 0;

protected:
    boost::asio::io_context& ctx_;
    SocketType socket_;
    AcceptorType acceptor_;

    AsioMsgHandlerFactory& factory_;

    ConnectionMapType connections_;

    mutable std::mutex mutex_; // mutable allows for ignoring CV in size() and empty()
    std::map<uint32_t, SignalScopedConnection> signalConnections_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_SERVER_H__
