/**
 Copyright © 2023 COMPAN REF
 @file company_ref_persistor_bl.h
 @brief Persistor Business Logic
 */
#ifndef __company_ref_PERSISTOR_BL_H__
#define __company_ref_PERSISTOR_BL_H__

#include <company_ref_boost_client/company_ref_boost_uds_client.h>
#include <company_ref_microservice/company_ref_microservice_client.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_variant.h>

#include "company_ref_persistor_client.h"

#include <string>
#include <vector>

namespace Compan{
namespace Edge {

class MicroServiceClient;
class VariantValueStore;

/*!
 * @brief	Persistor's business logic
 *
 * Uses a .ini to configure registration for notifications on value add's and changes
 * Stores changes to a persistant file (dmo) using a timer
 * Upon completion of the save, flushes the local VariantValueStore
 *
 * Config file should be as follows:
 * [persist]
 * file=path
 * flush_timeout=seconds
 *
 * [values]
 * valueid
 *
 */
class PersistorBl {
public:
    PersistorBl(boost::asio::io_context& ioContext, std::string const& udsPath, std::string const& cfgPath);

    virtual ~PersistorBl();

    /*!
     * Loads a .ini configuration
     * Registers for the values to notify of changes
     *
     * @param cfgPath   config file to load
     * @returns True on success load of a config file
     */
    bool load(std::string const& cfgPath);

protected:
    /// Notification the client has connected, start client pipeline
    void onConnected();

    /// Notification the client has disconnected, kill client pipeline
    void onDisconnected();

private:
    boost::asio::io_context& ioContext_; //!< Main execution context
    std::string udsPath_;                //!< uds socket path
    std::string cfgPath_;                //!< persistance config file

    MicroServiceClient client_; //!< Microservices client

    PersistorClient::Ptr persistorClient_;

    SignalScopedConnection clientConnectedListener;
    SignalScopedConnection clientDisconnectedListener;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_PERSISTOR_BL_H__
