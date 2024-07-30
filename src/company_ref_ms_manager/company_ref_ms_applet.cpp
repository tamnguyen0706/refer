/**
 Copyright © 2023 COMPAN REF
 @file company_ref_ms_applet.cpp
 @brief MicroService Application Manager object
 */
#include "company_ref_ms_applet.h"

#include <company_ref_variant_valuestore/company_ref_variant_bool_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_enum_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_text_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_uinterval_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_unorderedset_value.h>

#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <Compan_logger/Compan_logger.h>

#include <regex>

namespace Compan{
namespace Edge {

CompanLogger MicroServiceAppletLog("ms.applet", LogLevel::Information);

} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

MicroServiceApplet::MicroServiceApplet(
        boost::asio::io_context& ctx,
        VariantValueStore& ws,
        ValueId const& appletId,
        std::string const& udsPath)
    : ctx_(ctx)
    , ws_(ws)
    , appletId_(appletId)
    , udsPath_(udsPath)
    , restartTimeActive_(false)
    , restartTimer_(ctx_)
    , execParamsChanged_(true)
    , msValues_(ws_, appletId.parent(), appletId.leaf())
{
    FunctionArgLog(MicroServiceAppletLog) << appletId_ << std::endl;

    if (!msValues_.isValid()) {

        ErrorLog(MicroServiceAppletLog) << "Detected invalid microservice ValueIds for " << appletId_ << std::endl;
        return;
    }
}

void MicroServiceApplet::start()
{
    FunctionArgLog(MicroServiceAppletLog)
            << appletId_ << ", valid params:" << std::boolalpha << msValues_.isValid() << std::endl;

    if (!msValues_.isValid()) return;

    onParams_ = msValues_.config->params->connectChangedListener(
            std::bind(&MicroServiceApplet::onParams, shared_from_this(), std::placeholders::_1));

    onExecPath_ = msValues_.config->execPath->connectChangedListener(
            std::bind(&MicroServiceApplet::onExecPath, shared_from_this(), std::placeholders::_1));

    onEnabled_ = msValues_.config->enable->connectChangedListener(
            std::bind(&MicroServiceApplet::onEnabled, shared_from_this(), std::placeholders::_1));

    onDependsOn_ = msValues_.config->dependsOn->connectChangedListener(
            std::bind(&MicroServiceApplet::onDependsOn, shared_from_this(), std::placeholders::_1));

    onDependsOn(msValues_.config->dependsOn);
}

void MicroServiceApplet::stop()
{
    FunctionArgLog(MicroServiceAppletLog)
            << appletId_ << ", valid params:" << std::boolalpha << msValues_.isValid() << std::endl;

    if (!msValues_.isValid()) return;

    onParams_.disconnect();
    onExecPath_.disconnect();
    onDependsOn_.disconnect();
    onEnabled_.disconnect();

    stopProcess();
    processMonitor_.reset();

    restartTimer_.cancel();
}

void MicroServiceApplet::onEnabled(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(MicroServiceAppletLog) << appletId_ << std::endl;

    if (valuePtr == nullptr) return;

    if (!msValues_.config->enable->get()) {
        stopProcess();
        return;
    }

    // Check if the dependants have complete prior to starting this applet
    for (auto& dependents : dependsOnComplete_) {
        if (!dependents.first->get()) { return; }
    }

    stopProcess();

    if (execParamsChanged_) createProcessMonitor();

    startProcess();
}

void MicroServiceApplet::onDependsOn(VariantValue::Ptr const valuePtr)
{
    FunctionArgLog(MicroServiceAppletLog) << appletId_ << std::endl;

    if (valuePtr == nullptr) return;

    /// In the near future, this value will be a Set - so, we'd need to prune changes

    if (msValues_.config->dependsOn->empty()) {
        dependsOnComplete_.clear();

        onEnabled(msValues_.config->enable);
        return;
    }

    for (auto& key : msValues_.config->dependsOn->keys()) {

        // retrieve the whole object - will be thrown away
        MicroservicesValueIds::Microservices dependentMs(ws_, MicroservicesValueIds::BaseValueId, key);

        if (!dependentMs.isValid()) {
            ErrorLog(MicroServiceAppletLog) << "Failed to create microservice dependency" << std::endl;
            return;
        }

        // Get the startupComplete bool
        VariantBoolValue::Ptr startupComplete = dependentMs.status->startupComplete;

        // listen to it via onEnabled - which will make sure that all dependents are enabled
        SignalScopedConnection onDependsOn = startupComplete->connectChangedListener(
                std::bind(&MicroServiceApplet::onEnabled, shared_from_this(), std::placeholders::_1));

        dependsOnComplete_.emplace(startupComplete, std::move(onDependsOn));
    }
}

void MicroServiceApplet::onParams(VariantValue::Ptr const valuePtr)
{
    if (valuePtr == nullptr) return;
    execParamsChanged_ = true;
}

void MicroServiceApplet::onExecPath(VariantValue::Ptr const valuePtr)
{
    if (valuePtr == nullptr) return;
    execParamsChanged_ = true;
}

void MicroServiceApplet::getExecParams(std::string& execPath, std::vector<std::string>& paramList)
{
    FunctionArgLog(MicroServiceAppletLog) << appletId_ << std::endl;

    execPath = msValues_.config->execPath->str();

    for (auto& child : msValues_.config->params->getChildren()) {
        if (child.first == "uds" && child.second->str().empty()) {
            paramList.push_back("-u");
            paramList.push_back(udsPath_);
        } else {
            std::string param = child.second->str();

            std::regex split("\\s+");
            std::sregex_token_iterator iter(param.begin(), param.end(), split, -1);
            std::sregex_token_iterator end;
            for (; iter != end; ++iter) paramList.push_back(*iter);
        }
    }
}

void MicroServiceApplet::setRestartTimer()
{
    FunctionArgLog(MicroServiceAppletLog) << appletId_ << std::endl;

    if (restartTimeActive_) return;

    auto timeoutMs(msValues_.config->restartInterval->get());

    restartTimer_.expires_from_now(boost::posix_time::milliseconds(timeoutMs));
    restartTimer_.async_wait(
            std::bind(&MicroServiceApplet::restartTimerHandler, shared_from_this(), std::placeholders::_1));
    restartTimeActive_ = true;
}

void MicroServiceApplet::restartTimerHandler(boost::system::error_code const& error)
{
    FunctionArgLog(MicroServiceAppletLog) << appletId_ << std::endl;

    restartTimeActive_ = false;
    if (error) {
        ErrorLog(MicroServiceAppletLog) << error.message() << std::endl;
        return;
    }

    // start the timer to do it again
    onEnabled(msValues_.config->enable);
}

void MicroServiceApplet::processNotifier(CompanEdgeProcessMonitorBase* const processMonitor)
{
    FunctionArgLog(MicroServiceAppletLog) << appletId_ << std::endl;

    if (processMonitor == nullptr) {
        ErrorLog(MicroServiceAppletLog) << appletId_ << " null pointer?" << std::endl;
        return;
    }

    if (!msValues_.config->enable->get()) {
        if (!processMonitor->isRunning()) {
            msValues_.status->status->set(MicroservicesValueIds::Microservices::Status::Disabled);
            msValues_.status->running->set(false);
        }

        return;
    }

    ErrorLog(MicroServiceAppletLog) << appletId_ << " Ay, caramba! -> " << processMonitor->cmd()
                                    << " exited with return code: " << processMonitor->returnCode() << std::endl;

    msValues_.status->running->set(false);
    msValues_.status->status->set(
            processMonitor->returnCode() ? MicroservicesValueIds::Microservices::Status::Error :
                                           MicroservicesValueIds::Microservices::Status::Disabled);

    setRestartTimer();
}

void MicroServiceApplet::createProcessMonitor()
{
    FunctionArgLog(MicroServiceAppletLog) << appletId_ << std::endl;

    std::string execPath;
    std::vector<std::string> paramList;
    getExecParams(execPath, paramList);

    processMonitor_ = CompanEdgeProcessMonitoryFactory::make(
            ctx_,
            execPath,
            paramList,
            std::bind(&MicroServiceApplet::processNotifier, shared_from_this(), std::placeholders::_1));

    if (!processMonitor_) {
        ErrorLog(MicroServiceAppletLog) << appletId_ << " null pointer?" << std::endl;
        return;
    }

    execParamsChanged_ = false;
}

bool MicroServiceApplet::startProcess()
{
    FunctionArgLog(MicroServiceAppletLog) << appletId_ << std::endl;

    if (processMonitor_ == nullptr) return false;

    if (isProcessRunning()) return true;

    processMonitor_->start();

    if (isProcessRunning()) {
        msValues_.status->status->set(MicroservicesValueIds::Microservices::Status::Running);
        msValues_.status->running->set(true);

        msValues_.status->restartCount->inc(1);

        msValues_.status->startupComplete->set(false);

        return true;
    }

    return false;
}

void MicroServiceApplet::stopProcess()
{
    FunctionArgLog(MicroServiceAppletLog) << appletId_ << std::endl;

    if (processMonitor_ == nullptr) return;

    if (isProcessRunning()) processMonitor_->stop();

    // Reset the startup completed flag
    msValues_.status->startupComplete->set(false);
}

bool MicroServiceApplet::isProcessRunning()
{
    return (processMonitor_ && processMonitor_->isRunning());
}
