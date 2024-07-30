/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_server_connection_manager.h
  @brief CompanEdge Boost Server - connection manager
*/

#ifndef __company_ref_BOOST_SERVER_CONNECTION_MANAGER_H__
#define __company_ref_BOOST_SERVER_CONNECTION_MANAGER_H__

#include "company_ref_boost_server_connection.h"
#include <company_ref_utils/company_ref_signals.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

#include <memory>
#include <mutex>
#include <set>

namespace CompanEdgeProtocol {
class ServerMessage;
class ClientMessage;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {

template <typename T>
class CompanEdgeBoostServerConnectionManager {
public:
    using BoostServerConnectionPtr = std::shared_ptr<CompanEdgeBoostServerConnection<T>>;
    using ConnectionSet = std::set<BoostServerConnectionPtr>;
    using Iterator = typename ConnectionSet::const_iterator;
    ;

    using ConnectionAddSignal = SignalAsio<void(BoostServerConnectionPtr const)>;
    using ConnectionRemoveSignal = SignalAsio<void(BoostServerConnectionPtr const)>;

    CompanEdgeBoostServerConnectionManager(CompanEdgeBoostServerConnectionManager const&) = delete;
    CompanEdgeBoostServerConnectionManager& operator=(CompanEdgeBoostServerConnectionManager const&) = delete;

    CompanEdgeBoostServerConnectionManager(boost::asio::io_context& ctx);
    virtual ~CompanEdgeBoostServerConnectionManager() = default;

    // Add the specified connection to the manager and start it.
    void start(BoostServerConnectionPtr con);

    // Stop the specified connection.
    void stop(BoostServerConnectionPtr con);

    // Stop all connections.
    void stopAll();

    BoostServerConnectionPtr get(uint32_t const connectionId);

    SignalConnection connectConnectionAddedListener(typename ConnectionAddSignal::SlotType const&);
    SignalConnection connectConnectionRemovedListener(typename ConnectionRemoveSignal::SlotType const&);

    Iterator begin() const;
    Iterator end() const;

private:
    boost::asio::io_context& ctx_;

    // The managed connections.
    std::mutex mutex_;

    ConnectionSet connections_;

    ConnectionAddSignal onConnectonAddSignal_;
    ConnectionRemoveSignal onConnectonRemoveSignal_;
};

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_BOOST_SERVER_CONNECTION_MANAGER_H__*/
