/**
 Copyright © 2023 COMPAN REF
 @file company_ref_persistor_client.h
 @brief Uds Client for pipelineing the values to persistance
 */
#ifndef __company_ref_PERSISTOR_CLIENT_H__
#define __company_ref_PERSISTOR_CLIENT_H__

#include <company_ref_boost_client/company_ref_boost_uds_client.h>
#include <company_ref_protocol/company_ref_datamodel.pb.h>
#include <boost/asio/deadline_timer.hpp>

#include <memory>
#include <set>

namespace Compan{
namespace Edge {

struct ValueIdCompare {
    bool operator()(CompanEdgeProtocol::Value const& lhs, CompanEdgeProtocol::Value const& rhs) const
    {
        return lhs.id() < rhs.id();
    }
};

/*!
 * PersistorClient subscribes to value id's that are required
 * for persistance
 *
 * This bypasses the microservice client's ValueStore, keeping
 * values in memory only for the purpose of persistence
 */
class PersistorClient {
public:
    using Ptr = std::shared_ptr<PersistorClient>;

    PersistorClient(boost::asio::io_context& ioContext, std::string const& udsPath);
    virtual ~PersistorClient();

    void start(std::string const& cfgPath);

protected:
    /// Main message handler from the client
    void onMessageReceived(CompanEdgeProtocol::ClientMessage const& rspMsg);

    /// Value Changed notifications
    void onMessage(CompanEdgeProtocol::ValueChanged const& valueChanged);

    /// Value Removed notifications
    void onMessage(CompanEdgeProtocol::ValueRemoved const& valueRemoved);

    /// Value's added via the subscription
    void onMessage(CompanEdgeProtocol::VsResult const& vsResult);

    /// Sets the timer
    void setFlushTimer();

    /// Writes the data stored in the VariantValueStore to disk
    void flushTimerHandler(boost::system::error_code const& error);

    /*!
     * Loads the persisted DMO file
     *  - replaces values from the value update set
     *  - removes values from the remove id set
     *  - saves and clears update sets
     */
    void saveToFile();

    /// inserts value changes into the
    void insertValueChange(CompanEdgeProtocol::Value const& value);

private:
    CompanEdgeBoostUdsClient::Ptr udsClient_;  //!< UDS client to VariantValueStore server
    boost::asio::deadline_timer flushTimer_; //!< Timer to flush persistant values
    std::string persistPath_;                //!< Path to persist file
    uint32_t flushTimeoutSeconds_;           //!< Timer for persistent
    bool flushTimerActive_;                  //!< If the timer has been set

    SignalScopedConnection clientMessageConnection_;

    std::set<CompanEdgeProtocol::Value, ValueIdCompare> changeValues_; //!< Incoming values that have been updated
    std::set<std::string> removeIds_;                                //!< Keeps a copy the remove id's

    static std::string const PersistSectionName;
    static std::string const PersistFileName;
    static std::string const PersistFlushTimeoutName;
    static std::string const ValueSectionName;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_PERSISTOR_CLIENT_H__
