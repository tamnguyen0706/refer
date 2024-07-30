/**
  Copyright © 2023 COMPAN REF
  @file company_ref_boost_message_handler.cpp
  @brief CompanEdge Boost - message handler
*/

#ifndef __company_ref_BOOST_MESSAGE_HANDLER_H__
#define __company_ref_BOOST_MESSAGE_HANDLER_H__

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_hashtoken_map.h>

#include <company_ref_utils/company_ref_signals.h>
#include <functional>
#include <map>
#include <queue>
#include <set>

namespace Compan{
namespace Edge {

/*!
 * @brief Protocol Request handler
 *
 * Processes incoming message requests from client and
 * formulates response messages
 */
class CompanEdgeBoostMessageHandler {
public:
    using Ptr = std::shared_ptr<CompanEdgeBoostMessageHandler>;

    using ClientMessagePtr = std::shared_ptr<CompanEdgeProtocol::ClientMessage>;

    /// Callback function to signal message processing has completed
    using ClientMessageSignal = SignalAsio<void(ClientMessagePtr)>;

    CompanEdgeBoostMessageHandler(CompanEdgeBoostMessageHandler const&) = delete;
    CompanEdgeBoostMessageHandler& operator=(CompanEdgeBoostMessageHandler const&) = delete;

    /*!
     * Constructs a message handler object
     *
     * @param   Unique connection identifier
     */
    explicit CompanEdgeBoostMessageHandler(boost::asio::io_context& ctx, uint32_t const);

    virtual ~CompanEdgeBoostMessageHandler() = default;

    /// Message handlers could have their own internal connections
    /// Which need to be terminated when being shutdown, prior to destruction
    virtual void disconnect();

    /// Request message handler
    virtual void handleMessage(CompanEdgeProtocol::ServerMessage const& requestMessage) = 0;

    /// Sets the completion callback function
    SignalConnection connectClientMessageListener(ClientMessageSignal::SlotType const&);

protected:
    uint32_t const connectionId_;
    ClientMessageSignal onClientMessageSignal_;
};

inline SignalConnection CompanEdgeBoostMessageHandler::connectClientMessageListener(
        ClientMessageSignal::SlotType const& cb)
{
    return onClientMessageSignal_.connect(cb);
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_BOOST_MESSAGE_HANDLER_H__
