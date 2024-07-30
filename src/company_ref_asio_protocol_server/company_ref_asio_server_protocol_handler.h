/**
 Copyright © 2024 COMPAN REF
 @file company_ref_asio_server_protocol_handler.h
 @brief VariantValueStore base CompanEdgeProtocol message handler
 */
#ifndef __company_ref_ASIO_SERVER_PROTOCOL_HANDLER_H__
#define __company_ref_ASIO_SERVER_PROTOCOL_HANDLER_H__

#include <company_ref_utils/company_ref_callbacks.h>
#include <company_ref_utils/company_ref_signals.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_hashtoken_map.h>
#include <google/protobuf/repeated_field.h>
#include <mutex>

namespace CompanValueTypes {
class AddToContainer;
class RemoveFromContainer;
} // namespace CompanValueTypes

namespace CompanEdgeProtocol {
class ServerMessage;
class ClientMessage;
class Value;
class VsSync;
class VsSubscribe;
class VsUnsubscribe;
class VsGetAll;
class VsGetValue;
class VsSetValue;
class VsMultiGet;
class VsMultiSet;
class VsGetObject;
class ValueChanged;
class ValueRemoved;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {

class SignalScopedConnection;
class VariantValue;
class VariantValueStore;
class DmoContainer;

using ClientMessagePtr = std::shared_ptr<CompanEdgeProtocol::ClientMessage>;
using ServerMessagePtr = std::shared_ptr<CompanEdgeProtocol::ServerMessage>;
using VariantValuePtr = std::shared_ptr<VariantValue>;

/// Message handler that interfaces with VariantValueStore
class ServerProtocolHandler : public std::enable_shared_from_this<ServerProtocolHandler> {
public:
    using Ptr = std::shared_ptr<ServerProtocolHandler>;

    /// Callback function to signal message processing has completed
    using SendCallback = Signal<void(ClientMessagePtr)>;

    /*!
     * @param   Application's VariantValueStore object
     * @param   Response Message Queue to insert response messages
     * @param   Unique connection identifier
     */
    ServerProtocolHandler(VariantValueStore&, DmoContainer&, std::mutex&, uint32_t);
    virtual ~ServerProtocolHandler();

    virtual void disconnect();

    /// Request message handler
    void doHandleMessage(ServerMessagePtr requestMessage);

    /// Request message handler
    void doMessage(CompanEdgeProtocol::ServerMessage const& requestMessage);

    SignalConnection connectSendCallback(SendCallback::SlotType const& cb);

protected:
    ServerProtocolHandler(ServerProtocolHandler const&) = delete;
    ServerProtocolHandler& operator=(ServerProtocolHandler const&) = delete;

protected:
    // incoming ServerMessage handlers

    /*!
     * Subscribes to either a list of value.ids or all values
     *
     * Responds with VsSyncCompleted and a populated ValueChanged array of requested values
     *
     * @param VsSync request
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::VsSync const&);

    /*!
     * Event notification of a value being changed
     *
     * Unknown value id's are ignored
     *
     * Responds with a ValueChanged notification
     *
     * @param  ValueChanged notification
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::ValueChanged const&);

    /*!
     * Subscribes to either a list of value.ids or all values
     *
     * Subscriptions are cumulative until reset by a VsUnsubscribe request
     *
     * Responds with VsResult
     *
     * @param VsSubscribe request
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::VsSubscribe const&);

    /*!
     * Cancels all subscriptions
     *
     * Responds with VsResult
     *
     * @param VsUnsubscribe request
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::VsUnsubscribe const&);

    /*!
     * Retrieves all values from the Variant Value store
     *
     * Responds with VsResult
     *
     * @param VsGetAll request
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::VsGetAll const&);

    /*!
     * Retrieves a single value from the value store
     *
     * Responds with VsResult
     *
     * @param VsGetValue request
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::VsGetValue const&);

    /*!
     * Updates only the data portion of an existing value of Variant ValueStore Value.
     *
     * Responds with VsResult
     *
     * @param VsSetValue request
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::VsSetValue const&);

    /*!
     * Event notification of a value being removed by the client
     *
     * Responds with a ValueRemoved notification
     *
     * @param  ValueRemoved notification
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::ValueRemoved const&);

    /*!
     * Retrieves multiple values from the Variant ValueStore
     *
     * Responds with VsMultiGetResult
     *
     * @param VsMultiGet request
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::VsMultiGet const&);

    /*!
     * Updates only the data portion of multiple existing values in
     * the Variant ValueStore Value.
     *
     * Responds with VsMultiSetResult
     *
     * @param VsMultiSet request
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::VsMultiSet const&);

    /*!
     * Retrieves a value and it's children from the Variant ValueStore
     *
     * Responds with VsResult
     *
     * @param VsGetObject request
     */
    ClientMessagePtr doMessage(CompanEdgeProtocol::VsGetObject const&);

    // incoming Variant ValueStore handlers

    /*!
     * Value added callback handler from the Value Store
     *
     * Generates a ValueChanged notification
     *
     * @param VariantValuePtr with the added value
     */
    virtual void onValueAdded(VariantValuePtr const);

    /*!
     * Value changed callback handler from the Value Store
     *
     * Generates a ValueChanged notification
     *
     * @param VariantValuePtr with the changed value
     */
    virtual void onValueChanged(VariantValuePtr const);

    /*!
     * Value removed callback handler from the Value Store
     *
     * Generates a ValueRemoved notification
     *
     * @param VariantValuePtr with the removed value
     */
    virtual void onValueRemoved(VariantValuePtr const);

    virtual void onValueAddToContainer(VariantValuePtr const);
    virtual void onValueRemoveFromContainer(VariantValuePtr const);

private:
    void connectAllListeners();
    void connectAddRemoveListeners();
    void disconnectListeners();

    bool doAddToContainer(std::string const&, CompanValueTypes::AddToContainer const&);
    bool doRemoveFromContainer(std::string const&, CompanValueTypes::RemoveFromContainer const&);

    /// Checks if the VariantValue or it's parent is in the subscriber list
    bool isParentSubscribed(VariantValuePtr const valuePtr);

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
            VariantValuePtr valuePtr,
            google::protobuf::RepeatedPtrField<CompanEdgeProtocol::Value>* repeatedValues,
            bool const subscribe = false);

    /// Inserts the VariantValuePtr into the subscription connection filter
    void insertSubscriberFilter(VariantValuePtr valuePtr);

private:
    VariantValueStore& ws_;
    DmoContainer& dmo_;
    std::mutex& wsContainerMutex_; // used to make sure two connections aren't attempting to
                                   // add/remove elements

    uint32_t const connectionId_;
    SendCallback onSendCallback_;

    // map of hashToken and value changed signal for discrete value connection
    HashTokenMap<SignalScopedConnection> wsSubscriberConnection_;

    // listen for Variant ValueStore global connections
    SignalScopedConnection addedListener_;
    SignalScopedConnection changedListener_;
    SignalScopedConnection removedListener_;

    SignalScopedConnection addToContainerListener_;
    SignalScopedConnection removeFromContainerListener_;
};

inline SignalConnection ServerProtocolHandler::connectSendCallback(SendCallback::SlotType const& cb)
{
    return onSendCallback_.connect(cb);
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_ASIO_SERVER_PROTOCOL_HANDLER_H__
