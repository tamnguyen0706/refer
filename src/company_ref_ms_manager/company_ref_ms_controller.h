/**
 Copyright © 2023 COMPAN REF
 @file company_ref_ms_controller.h
 @brief Main controlling interface
 */
#ifndef __company_ref_MS_CONTROLLER_H__
#define __company_ref_MS_CONTROLLER_H__

#include "company_ref_ms_applet.h"

#include <company_ref_boost_client/company_ref_boost_uds_client.h>
#include <company_ref_microservice/company_ref_microservice_client.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>
#include <boost/asio/io_context.hpp>

namespace Compan{
namespace Edge {

/*!
 * Main interface for the MicroService manager
 *
 * - Handles subscribing to the system.microservices container
 * - Creates controlling applet objects to manage the runtime
 *   of a microservice
 */
class MicroServiceController {
public:
    MicroServiceController(boost::asio::io_context& ioContext, std::string const& udsPath);
    virtual ~MicroServiceController();

    void onMicroServices(VariantValue::Ptr const valuePtr);

protected:
    /// callback for when subscription to the system.microservices container is complete
    void subscriptionCompleted();

    void disconnectApplets();

private:
    boost::asio::io_context& ioContext_; //!< Main execution context
    std::string const& udsPath_;         //!< uds Socket path

    MicroServiceClient client_; //!< Microservices client

    std::vector<MicroServiceApplet::Ptr> applets_; //!< Applet handlers
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_MS_CONTROLLER_H__
