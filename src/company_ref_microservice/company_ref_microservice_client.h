/**
 Copyright © 2023 COMPAN REF
 @file company_ref_microservice_client.cpp
 @brief MicroService Client
 */
#ifndef __company_ref_MICROSERVICE__CLIENT_H__
#define __company_ref_MICROSERVICE__CLIENT_H__

#include "company_ref_microservice_connection.h"
#include "company_ref_microservice_message_handler.h"
#include <company_ref_asio/company_ref_asio_connection.h>
#include <company_ref_sdk_value_ids/dynamic_dmo_value_ids.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_variant.h>

#include <functional>
#include <queue>

namespace CompanEdgeProtocol {
class ServerMessage;
class ClientMessage;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {

// fwd decls
class VariantValueStore;
class MicroserviceDynamicDmo;
class MicroservicePersistedDmo;

/*!
 * @brief MicroService Client
 *
 * MicroService Client handles the subscription aspect of communication to a
 * VariantValueStore Server.
 *
 * Values can either be stored in the application's VariantValueStore locally,
 * or signaled via pass thru callbacks.
 *
 * This allows a client to either store the data or simply pass it on to another
 * application.
 */
class MicroServiceClient {
public:
    using SubscribeCompleteCb = std::function<void(bool const)>;
    using LoadDmoCompleteCb = std::function<void(DynamicDmoValueIds::Dmo::Status::ActionEnum const)>;
    using LoadPersistedCompleteCb = std::function<void(DynamicDmoValueIds::Dmo::Persisted::Status::ActionEnum const)>;

    MicroServiceClient(boost::asio::io_context& ctx, std::string const& appName, std::string const& udsPath);
    MicroServiceClient(
            boost::asio::io_context& ctx,
            std::string const& appName,
            std::string const& address,
            std::string const& port);
    virtual ~MicroServiceClient();

    /// Returns the app name of the client
    std::string appName() const;

    /// Returns the Boost connection client
    AsioConnectionBase::Ptr clientConnection();

    /// Returns the local VariantValueStore
    VariantValueStore& ws();

    /// Returns a Dynamic Dmo interface object
    std::shared_ptr<MicroserviceDynamicDmo> getDynamicDmo();

    /// Returns a Persisted Dmo interface object
    std::shared_ptr<MicroservicePersistedDmo> getPersistedDmo(std::string const&);

    /// Whether to auto reconnect to the ws server (default is true)
    void autoReconnect(bool const arg);

    /*!
     * Subscribes to individual value id's from the ValueStore Server
     *
     * This will create a VariantValue in the local value store, and connect to a subscription
     * completed callback
     *
     * @param valueId   Valid Value Id to subscribe to
     * @param cb        Callback handler for change notifications
     * @returns Callback subscription token
     */
    int subscribeWithCompletion(std::string const& valueId, SubscribeCompleteCb const& cb);

    /*!
     * Subscribes to multiple value id's from the ValueStore Server
     *
     * This will create a VariantValue in the local value store, and connect to a subscription
     * completed callback
     *
     * @param valueId   Valid Value Id to subscribe to
     * @param cb        Callback handler for change notifications
     * @returns Callback subscription token
     */
    int subscribeWithCompletion(std::vector<std::string> const& valueIds, SubscribeCompleteCb const& cb);

    /*!
     * Subscribes to ALL values from the ValueStore Server
     *
     * This will inserts all values into the local value store
     * @param cb         Callback handler for subscribe complete notifications
     * * @returns Callback subscription token
     */
    int subscribeAll(SubscribeCompleteCb const& cb = nullptr);

    /*!
     * Cancels a waiting subscription
     * @param subscription
     */
    void cancelSubscibeCompletion(int const subscription);

    /*!
     * Unsubscribes all values from the ValueStore Server
     * This function will also remove values stored in the local VariantValueStore
     */
    void unsubscribeAll();

    /// inserts a message into the outgoing queue
    void insertWriteQueue(CompanEdgeProtocol::ServerMessage&& msg);

    /// Allows the microservice to signal it's start up is complete
    void setStartupCompleted();

protected:
    MicroServiceClient(boost::asio::io_context& ctx, std::string const& appName);

    void clientConnection(AsioConnectionBase::Ptr);

    void onClientConnected();
    void onClientDisconnected();

    void onMessageReceived(CompanEdgeProtocol::ClientMessage const&);

    void writeFromQueue();

    void subscribeComplete(CompanEdgeProtocol::VsResult const&);

    void onMicroserviceSubcriptionComplete(bool const completed);

    // for unit testing
    MicroServiceMessageHandler::Ptr messageHandler();

private:
    boost::asio::io_context& ctx_;
    std::string const appName_;
    VariantValueStore ws_;
    bool autoReconnect_;
    uint32_t seqNo_;
    MicroServiceMessageHandler::Ptr messageHandler_;

    AsioConnectionBase::Ptr client_;
    ClientConnectionManager::Ptr connectionManager_;

    SignalScopedConnection clientConnectedConnection_;
    SignalScopedConnection clientDisconnectedConnection_;
    SignalScopedConnection clientMessageConnection_;
    bool connected_;
    std::map<uint32_t, SubscribeCompleteCb> subscribeCompleteMapCb_;

    SignalScopedConnection onSubscribeComplete_;

    std::shared_ptr<MicroserviceDynamicDmo> dynamicDmo_;

    std::queue<CompanEdgeProtocol::ServerMessage> requestQueue_;
};

inline std::string MicroServiceClient::appName() const
{
    return appName_;
}

inline AsioConnectionBase::Ptr MicroServiceClient::clientConnection()
{
    return client_;
}

inline VariantValueStore& MicroServiceClient::ws()
{
    return ws_;
}

inline MicroServiceMessageHandler::Ptr MicroServiceClient::messageHandler()
{
    return messageHandler_;
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_MICROSERVICE__CLIENT_H__
