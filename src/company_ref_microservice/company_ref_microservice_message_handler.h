/**
 Copyright © 2023 COMPAN REF
 @file company_ref_microservice_message_handler.h
 @brief Microservice Client Message Handler
 */
#ifndef __company_ref_MICROSERVICE_MESSAGE_HANDLER__
#define __company_ref_MICROSERVICE_MESSAGE_HANDLER__

#include <company_ref_asio_protocol_client/company_ref_asio_client_protocol_handler.h>

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_utils/company_ref_signals.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_hashtoken_set.h>

#include <map>
#include <queue>

namespace Compan{
namespace Edge {

class VariantValueStore;

/*!
 * @brief Message handler for MicroService client
 *
 * Responds to CompanEdgeProtocol::ClientMessage sent from a CompanVariantValueStore server.
 *
 * Adds, Removes and changes value stored in a local VariantValueStore object.
 *
 * Adding a CompanEdgeProtocol::Unset value to the VariantValueStore will cause
 * this class to subscribe for change notifications from the connected VariantValueStore
 * server.
 *
 * Changes to values in the local VariantValueStore will be transmitted to the VariantValueStore
 * server.
 */
class MicroServiceMessageHandler : public ClientProtocolHandler {
public:
    using Ptr = std::shared_ptr<MicroServiceMessageHandler>;

    using ValueRemovedNotification = std::function<void(std::string const&)>;
    using SubscribeSignal = Signal<void(CompanEdgeProtocol::VsResult const&)>;

    MicroServiceMessageHandler(MicroServiceMessageHandler const&) = delete;
    MicroServiceMessageHandler& operator=(MicroServiceMessageHandler const&) = delete;

public:
    MicroServiceMessageHandler(VariantValueStore& ws, std::string const& appName, uint32_t& seqNo);
    virtual ~MicroServiceMessageHandler();

    virtual void disconnect();

    /// Request message handler
    virtual void doMessage(CompanEdgeProtocol::ClientMessage const& responseMessage);

    SignalConnection connectSubscribeListener(SubscribeSignal::SlotType const& cb);

    void write(CompanEdgeProtocol::ServerMessage const&);

    /// Filters to keep from cross pollution of values
    void insertAppNameFilter(ValueId const&);

    void removeAppNameFilter(ValueId const&);

protected:
    void queueRequestMessage(CompanEdgeProtocol::ServerMessage&& requestMessage);

    void onMessage(CompanEdgeProtocol::ValueChanged const& valueChanged);
    void onMessage(CompanEdgeProtocol::ValueRemoved const& valueRemoved);
    void onMessage(CompanEdgeProtocol::VsResult const& vsResult);
    void onMessage(CompanEdgeProtocol::VsSyncCompleted const& VsSyncCompleted);

    // incoming Variant ValueStore handlers

    /*!
     * Value added callback handler from the Value Store
     *
     * Generates a VsSubscribe message if the service is doing discreet subscriptions
     *
     * @param VariantValue::Ptr with the added value
     */
    void handleValueAdded(VariantValue::Ptr const);

    /*!
     * Value changed callback handler from the Value Store
     *
     * Generates a ValueChanged notification
     *
     * @param VariantValue::Ptr with the changed value
     */
    void handleValueChanged(VariantValue::Ptr const);

    /*!
     * Value removed callback handler from the Value Store
     *
     * Generates a ValueRemoved notification
     *
     * @param VariantValue::Ptr with the removed value
     */
    void handleValueRemoved(VariantValue::Ptr const);

    /*!
     * Updates the variant value in the value store
     * @param value Value object
     * @return  True on successful update
     */
    bool valueUpdate(CompanEdgeProtocol::Value const& value);

    bool isAppNameFiltered(ValueId const& valueId);
    bool isAppNameMatch(ValueId const& valueId);

private:
    VariantValueStore& ws_;
    std::string appName_;
    uint32_t& seqNo_;

    HashTokenSet parentFilter_;

    SubscribeSignal subscribeSignal_;

    // listen for Variant ValueStore global connections
    SignalScopedConnection addedListener_;
    SignalScopedConnection changedListener_;
    SignalScopedConnection removedListener_;

    SignalScopedConnection addToContainerListener_;
    SignalScopedConnection removeFromContainerListener_;
};

inline SignalConnection MicroServiceMessageHandler::connectSubscribeListener(SubscribeSignal::SlotType const& cb)
{
    return subscribeSignal_.connect(cb);
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_MICROSERVICE_MESSAGE_HANDLER__
