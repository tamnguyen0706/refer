/**
 Copyright © 2023 COMPAN REF
 @file company_ref_boost_ws_message_handler.h
 @brief VariantValueStore based message handler
 */
#ifndef __company_ref_BOOST_WS_MESSAGE_HANDLER_H__
#define __company_ref_BOOST_WS_MESSAGE_HANDLER_H__

#include "company_ref_boost_message_handler.h"

namespace Compan{
namespace Edge {

/// Message handler that interfaces with VariantValueStore
class CompanEdgeBoostWsMessageHandler : public CompanEdgeBoostMessageHandler,
                                      public std::enable_shared_from_this<CompanEdgeBoostWsMessageHandler> {
public:
    /*!
     * @param   Application's VariantValueStore object
     * @param   Response Message Queue to insert response messages
     * @param   Unique connection identifier
     */
    CompanEdgeBoostWsMessageHandler(boost::asio::io_context& ctx, VariantValueStore&, DmoContainer&, uint32_t);
    virtual ~CompanEdgeBoostWsMessageHandler();

    virtual void disconnect();

    /// Request message handler
    virtual void handleMessage(CompanEdgeProtocol::ServerMessage const& requestMessage);

protected:
    CompanEdgeBoostWsMessageHandler(CompanEdgeBoostWsMessageHandler const&) = delete;
    CompanEdgeBoostWsMessageHandler& operator=(CompanEdgeBoostWsMessageHandler const&) = delete;

protected:
    // incoming ServerMessage handlers

    /*!
     * Subscribes to either a list of value.ids or all values
     *
     * Responds with VsSyncCompleted and a populated ValueChanged array of requested values
     *
     * @param VsSync request
     */
    virtual void onMessage(CompanEdgeProtocol::VsSync const&, ClientMessagePtr);

    /*!
     * Event notification of a value being changed
     *
     * Unknown value id's are ignored
     *
     * Responds with a ValueChanged notification
     *
     * @param  ValueChanged notification
     */
    virtual void onMessage(CompanEdgeProtocol::ValueChanged const&, ClientMessagePtr);

    /*!
     * Subscribes to either a list of value.ids or all values
     *
     * Subscriptions are cumulative until reset by a VsUnsubscribe request
     *
     * Responds with VsResult
     *
     * @param VsSubscribe request
     */
    virtual void onMessage(CompanEdgeProtocol::VsSubscribe const&, ClientMessagePtr);

    /*!
     * Cancels all subscriptions
     *
     * Responds with VsResult
     *
     * @param VsUnsubscribe request
     */
    virtual void onMessage(CompanEdgeProtocol::VsUnsubscribe const&, ClientMessagePtr);

    /*!
     * Retrieves all values from the Variant Value store
     *
     * Responds with VsResult
     *
     * @param VsGetAll request
     */
    virtual void onMessage(CompanEdgeProtocol::VsGetAll const&, ClientMessagePtr);

    /*!
     * Retrieves a single value from the value store
     *
     * Responds with VsResult
     *
     * @param VsGetValue request
     */
    virtual void onMessage(CompanEdgeProtocol::VsGetValue const&, ClientMessagePtr);

    /*!
     * Updates only the data portion of an existing value of Variant ValueStore Value.
     *
     * Responds with VsResult
     *
     * @param VsSetValue request
     */
    virtual void onMessage(CompanEdgeProtocol::VsSetValue const&, ClientMessagePtr);

    /*!
     * Event notification of a value being removed by the client
     *
     * Responds with a ValueRemoved notification
     *
     * @param  ValueRemoved notification
     */
    virtual void onMessage(CompanEdgeProtocol::ValueRemoved const&, ClientMessagePtr);

    /*!
     * Retrieves multiple values from the Variant ValueStore
     *
     * Responds with VsMultiGetResult
     *
     * @param VsMultiGet request
     */
    virtual void onMessage(CompanEdgeProtocol::VsMultiGet const&, ClientMessagePtr);

    /*!
     * Updates only the data portion of multiple existing values in
     * the Variant ValueStore Value.
     *
     * Responds with VsMultiSetResult
     *
     * @param VsMultiSet request
     */
    virtual void onMessage(CompanEdgeProtocol::VsMultiSet const&, ClientMessagePtr);

    /*!
     * Retrieves a value and it's children from the Variant ValueStore
     *
     * Responds with VsResult
     *
     * @param VsGetObject request
     */
    virtual void onMessage(CompanEdgeProtocol::VsGetObject const&, ClientMessagePtr);

    // incoming Variant ValueStore handlers

    /*!
     * Value added callback handler from the Value Store
     *
     * Generates a ValueChanged notification
     *
     * @param VariantValue::Ptr with the added value
     */
    virtual void handleValueAdded(VariantValue::Ptr const);

    /*!
     * Value changed callback handler from the Value Store
     *
     * Generates a ValueChanged notification
     *
     * @param VariantValue::Ptr with the changed value
     */
    virtual void handleValueChanged(VariantValue::Ptr const);

    /*!
     * Value removed callback handler from the Value Store
     *
     * Generates a ValueRemoved notification
     *
     * @param VariantValue::Ptr with the removed value
     */
    virtual void handleValueRemoved(VariantValue::Ptr const);

    virtual void handleValueAddToContainer(VariantValue::Ptr const);
    virtual void handleValueRemoveFromContainer(VariantValue::Ptr const);

private:
    void connectAllListeners();
    void connectAddRemoveListeners();
    void disconnectListeners();

    bool handleAddToContainer(std::string const&, AddToContainer const&);
    bool handleRemoveFromContainer(std::string const&, RemoveFromContainer const&);

    /// Checks if the VariantValue or it's parent is in the subscriber list
    bool isParentSubscribed(VariantValue::Ptr const valuePtr);

    /// Copies the VariantValueStore elements into a repeated field accepting CompanEdgeProtocol::Value
    void copyValueStoreToRepeated(google::protobuf::RepeatedPtrField<CompanEdgeProtocol::Value>*);

    /*!
     * Walks the children of a value and adds it to the RepeatedPtrField
     *
     * If doing a discrete subscription, adds it to the  wsSubscriberConnection_
     * @param valuePtr          Value to walk the child of
     * @param repeatedValues    RepeatedPtrField to add values to
     * @param subscribe         Whether to add the value to wsSubscriberConnection_
     */
    void copyValueChildrenToRepeated(
            VariantValue::Ptr valuePtr,
            google::protobuf::RepeatedPtrField<CompanEdgeProtocol::Value>* repeatedValues,
            bool const subscribe = false);

    /// Inserts the VariantValue::Ptr into the subscription connection filter
    void insertSubscriberFilter(VariantValue::Ptr valuePtr);

private:
    VariantValueStore& variantValueStore_;
    DmoContainer& dmo_;

    // map of hashToken and value changed signal for discrete value connection
    HashTokenMap<SignalScopedConnection> wsSubscriberConnection_;

    // listen for Variant ValueStore global connections
    SignalScopedConnection addedListener_;
    SignalScopedConnection changedListener_;
    SignalScopedConnection removedListener_;

    SignalScopedConnection addToContainerListener_;
    SignalScopedConnection removeFromContainerListener_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_BOOST_WS_MESSAGE_HANDLER_H__
