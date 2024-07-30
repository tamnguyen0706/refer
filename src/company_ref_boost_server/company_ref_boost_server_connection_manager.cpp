/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_server_connection_manager.cpp
  @brief CompanEdge Boost Server - connection manager
*/

#include "company_ref_boost_server_connection_manager.h"
#include "company_ref_boost_server_connection.h"

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/protobuf.h>
#include <Compan_logger/Compan_logger.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>

using namespace Compan::Edge;

namespace Compan{
namespace Edge {
CompanLogger AebServerConnectionManagerLog("server.connection.manager", LogLevel::Information);
} // namespace Edge
} // namespace Compan

template <typename T>
CompanEdgeBoostServerConnectionManager<T>::CompanEdgeBoostServerConnectionManager(boost::asio::io_context& ctx)
    : ctx_(ctx)
    , onConnectonAddSignal_(ctx)
    , onConnectonRemoveSignal_(ctx)
{
}

template <typename T>
void CompanEdgeBoostServerConnectionManager<T>::start(BoostServerConnectionPtr con)
{
    FunctionArgLog(AebServerConnectionManagerLog);

    if (!con) return;

    std::lock_guard<std::mutex> lock(mutex_);
    connections_.insert(con);
    onConnectonAddSignal_(con);
    con->connectDisconnectListener(std::bind(&CompanEdgeBoostServerConnectionManager::stop, this, std::placeholders::_1));
    con->start();

    if (connections_.size() > 5)
        WarnLog(AebServerConnectionManagerLog) << "warning: " << connections_.size() << std::endl;
}

template <typename T>
void CompanEdgeBoostServerConnectionManager<T>::stop(BoostServerConnectionPtr con)
{
    FunctionLog(AebServerConnectionManagerLog);

    if (!con) return;

    std::lock_guard<std::mutex> lock(mutex_);
    if (connections_.count(con) == 0) return;

    connections_.erase(con);
    onConnectonRemoveSignal_(con);

    con->stop();
}

template <typename T>
void CompanEdgeBoostServerConnectionManager<T>::stopAll()
{
    FunctionLog(AebServerConnectionManagerLog);

    for (auto con : connections_) con->stop();

    connections_.clear();
}

template <typename T>
SignalConnection CompanEdgeBoostServerConnectionManager<T>::connectConnectionAddedListener(
        typename ConnectionAddSignal::SlotType const& cb)
{
    return onConnectonAddSignal_.connect(cb);
}

template <typename T>
SignalConnection CompanEdgeBoostServerConnectionManager<T>::connectConnectionRemovedListener(
        typename ConnectionRemoveSignal::SlotType const& cb)
{
    return onConnectonRemoveSignal_.connect(cb);
}

template <typename T>
typename CompanEdgeBoostServerConnectionManager<T>::BoostServerConnectionPtr CompanEdgeBoostServerConnectionManager<T>::get(
        uint32_t const connectionId)
{
    for (auto con : connections_) {
        if (con->getConnectionId() == connectionId) return con;
    }

    return typename CompanEdgeBoostServerConnectionManager<T>::BoostServerConnectionPtr();
}

template <typename T>
typename CompanEdgeBoostServerConnectionManager<T>::Iterator CompanEdgeBoostServerConnectionManager<T>::begin() const
{
    return connections_.begin();
}

template <typename T>
typename CompanEdgeBoostServerConnectionManager<T>::Iterator CompanEdgeBoostServerConnectionManager<T>::end() const
{
    return connections_.end();
}

template class COMPAN::REF::CompanEdgeBoostServerConnectionManager<boost::asio::ip::tcp>;
template class COMPAN::REF::CompanEdgeBoostServerConnectionManager<boost::asio::local::stream_protocol>;
