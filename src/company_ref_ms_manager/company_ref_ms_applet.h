/**
 Copyright © 2023 COMPAN REF
 @file company_ref_ms_applet.h
 @brief MicroService Application Manager object
 */
#ifndef __company_ref_MICROSERVICE_APPLET_H__
#define __company_ref_MICROSERVICE_APPLET_H__

#include <company_ref_sdk_value_ids/microservices_value_ids.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>
#include <boost/asio/io_context.hpp>

#include <company_ref_utils/company_ref_native_process_monitor.h>
#include <company_ref_utils/company_ref_process_monitor.h>
#include <company_ref_utils/company_ref_process_monitor_factory.h>

#include <memory>

namespace Compan{
namespace Edge {

/*!
 * MicroService Application Manager
 */
class MicroServiceApplet : public std::enable_shared_from_this<MicroServiceApplet> {
public:
    using Ptr = std::shared_ptr<MicroServiceApplet>;

    MicroServiceApplet(
            boost::asio::io_context& ctx,
            VariantValueStore& ws,
            ValueId const& appletId,
            std::string const& udsPath);
    virtual ~MicroServiceApplet() = default;

    ValueId appletId() const;

    /// Starts the applet handler
    void start();

    /// Gracefully tears everything down
    void stop();

    /// change handler for system.microservices.+.Config.enable
    void onEnabled(VariantValue::Ptr const valuePtr);

    /// change handler for system.microservices.+.Config.dependsOn
    void onDependsOn(VariantValue::Ptr const valuePtr);

    /// change handler for system.microservices.+.Config.params
    void onParams(VariantValue::Ptr const valuePtr);

    /// change handler for system.microservices.+.Config.execPath
    void onExecPath(VariantValue::Ptr const valuePtr);

protected:
    /// attempts to start the process - updates valuestore values
    void launchProcess();

    /// Retrieves the exec path and param string for CompanEdgeProcessMonitorBase
    void getExecParams(std::string& execPath, std::vector<std::string>& paramList);

    /// Creates the ProcessMonitor object
    void createProcessMonitor();

    /// Starts ProcessMonitor object
    bool startProcess();

    /// Stops ProcessMonitor object
    void stopProcess();

    /// Checks if the process is running
    bool isProcessRunning();

    /// Sets the timer to restart a dead process
    void setRestartTimer();

    /// Attempts to restart the process that died
    void restartTimerHandler(boost::system::error_code const& error);

    /// Notifier callback from CompanEdgeProcessMonitorBase
    void processNotifier(CompanEdgeProcessMonitorBase* const);

private:
    boost::asio::io_context& ctx_;
    VariantValueStore& ws_;
    ValueId const appletId_;
    std::string const& udsPath_;
    bool restartTimeActive_;
    boost::asio::deadline_timer restartTimer_;

    bool execParamsChanged_; // whether we need to recreate the processMonitor_ object

    std::shared_ptr<CompanEdgeProcessMonitorBase> processMonitor_;

    SignalScopedConnection onEnabled_;
    SignalScopedConnection onDependsOn_;

    SignalScopedConnection onParams_;
    SignalScopedConnection onExecPath_;

    MicroservicesValueIds::Microservices msValues_;

    // List of dependent microservices we need to wait for completion on
    std::map<VariantBoolValuePtr, SignalScopedConnection> dependsOnComplete_;
};

inline ValueId MicroServiceApplet::appletId() const
{
    return appletId_;
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_MICROSERVICE_APPLET_H__
