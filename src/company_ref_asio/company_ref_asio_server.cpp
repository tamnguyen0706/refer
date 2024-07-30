/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_server.cpp
 @brief Agnostic Socket type connection server
 */
#include "company_ref_asio_server.h"
#include "company_ref_asio_connection.h"
#include "company_ref_asio_msg_handler_factory.h"

#include <Compan_logger/Compan_logger.h>

using namespace Compan::Edge;

CompanLogger AsioServerLog("asio.server", LogLevel::Information);

template <typename T>
AsioServer<T>::AsioServer(boost::asio::io_context& ctx, AsioMsgHandlerFactory& factory)
    : ctx_(ctx)
    , socket_(ctx_)
    , acceptor_(ctx_)
    , factory_(factory)
{
}

template <typename T>
AsioServer<T>::~AsioServer()
{
    close();
}

template <typename T>
void AsioServer<T>::close()
{
    signalConnections_.clear();

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& conn : connections_) conn.second->close();

    connections_.clear();
}

template <typename T>
AsioConnectionPtr AsioServer<T>::get(uint32_t const connectionId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (connections_.count(connectionId)) return connections_[connectionId];

    return nullptr;
}

template <typename T>
typename AsioServer<T>::ConnectionMapType::const_iterator AsioServer<T>::begin()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return connections_.begin();
}

template <typename T>
typename AsioServer<T>::ConnectionMapType::const_iterator AsioServer<T>::end()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return connections_.end();
}

template <typename T>
bool AsioServer<T>::empty() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return connections_.empty();
}

template <typename T>
void AsioServer<T>::doAccept(typename SocketType::endpoint_type const& endpoint)
{
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(typename T::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    doAccept();
}

template <typename T>
void AsioServer<T>::doAccept()
{
    acceptor_.async_accept([this](boost::system::error_code ec, SocketType socket) {
        if (ec) { return; }

        createConnection(std::move(socket));
        doAccept();
    });
}

template <typename T>
void AsioServer<T>::onDisconnection(uint32_t const connectionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.erase(connectionId);
    signalConnections_.erase(connectionId);
}

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>

template class COMPAN::REF::AsioServer<boost::asio::ip::tcp>;
template class COMPAN::REF::AsioServer<boost::asio::local::stream_protocol>;
