/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_factory.cpp
 @brief Creates CompanLogger Binders to the ValueStore
 */
#include "Compan_logger_factory.h"
#include "Compan_logger_binder_variantvalue.h"

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_variant_valuestore/company_ref_variant_container_util.h>
#include <company_ref_variant_valuestore/company_ref_variant_enum_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>
#include <Compan_logger/Compan_logger.h>
#include <Compan_logger/Compan_logger_controller.h>
#include <Compan_logger/Compan_logger_loglevel.h>

#include <iostream>

///
#include <company_ref_protocol_utils/company_ref_stream.h>
///

namespace Compan{
namespace Edge {

/*!
 * This class is either used locally by the VariantValueStore server directly,
 * or in the case of a Microservice, waits the value id "logger" to become available
 *
 * This class is a fire and forget object, which self destructs once the creation of
 * binders is complete
 */
class LoggerFactoryAutoRegister : public std::enable_shared_from_this<LoggerFactoryAutoRegister> {
public:
    using Ptr = std::shared_ptr<LoggerFactoryAutoRegister>;

    LoggerFactoryAutoRegister(std::string const& appName, VariantValueStore& ws, bool const isMicroService)
        : appName_(appName)
        , ws_(ws)
        , isMicroService_(isMicroService)
        , appLoggerKey_(LoggerFactory::LoggerId, appName_)
    {
        CompanEdgeProtocol::Value logger;

        valueInit(logger, CompanEdgeProtocol::Container);
        logger.set_id(LoggerFactory::LoggerId);

        ws_.set(logger, VariantValue::Remote);

        loggerMap_ = ws_.get<VariantMapValue>(LoggerFactory::LoggerId);
    }

    virtual ~LoggerFactoryAutoRegister() = default;

    /// Registers slots for Value Added or Changed
    void registerLogger()
    {
        if (!loggerMap_->insert(appName_)) {
            std::cerr << "LoggerFactory::registerLogger -> Failed in insert " << appName_ << std::endl;
            return;
        }

        onAddToLoggerMap_ = loggerMap_->connectValueAddToContainerListener(
                std::bind(&LoggerFactoryAutoRegister::onAddToLoggerMap, shared_from_this(), std::placeholders::_1));
    }

    void onAddToLoggerMap(VariantValue::Ptr const loggerPtr)
    {
        if (loggerPtr->setUpdateType() == VariantValue::Local) return;

        onAddToLoggerMap_.disconnect();

        loggerAppMap_ = ws_.get<VariantMapValue>(appLoggerKey_);
        if (loggerAppMap_ == nullptr) return;

        onAddToLoggerAppMap_ = loggerAppMap_->connectValueAddToContainerListener(
                std::bind(&LoggerFactoryAutoRegister::onAddToLoggerAppMap, shared_from_this(), std::placeholders::_1));

        // Walks the registered list of CompanLogger elements (sources)
        for (auto& source : CompanLoggerController::get()->sources()) {

            ValueId monitorKey(appLoggerKey_, source->name());

            VariantValue::Ptr valuePtr = ws_.get(monitorKey);

            if (valuePtr != nullptr) {
                bindSource(valuePtr, source);
                continue;
            }

            if (!loggerAppMap_->insert(source->name())) {
                std::cerr << "LoggerFactory::onAddToLoggerMap -> Failed in insert " << source->name() << std::endl;
                continue;
            }

            loggerIds_.emplace(monitorKey, source);
        }

        loggerMap_.reset();

        // don't need to listen for any more changes
        if (loggerIds_.empty()) onAddToLoggerAppMap_.disconnect();
    }

    void onAddToLoggerAppMap(VariantValue::Ptr const loggerPtr)
    {
        if (loggerPtr->setUpdateType() == VariantValue::Local) return;

        ValueId monitorKey(loggerPtr->id(), VariantContainerUtil::getKey(loggerPtr));

        VariantValue::Ptr valuePtr = ws_.get(monitorKey);

        if (valuePtr == nullptr) return;

        // register
        bindSource(valuePtr, loggerIds_[monitorKey]);

        // all done with this element, remove it from the wait list
        loggerIds_.erase(monitorKey);

        // don't need to listen for any more changes
        if (loggerIds_.empty()) onAddToLoggerAppMap_.disconnect();
    }

    void bindSource(VariantValue::Ptr valuePtr, CompanLogger* source)
    {
        if (valuePtr == nullptr || source == nullptr) return;

        // Create the Binder object that has the VariantValue and CompanLogger combined
        VariantValueLoggerBinder::Ptr monitor(std::make_unique<VariantValueLoggerBinder>(valuePtr, *source));

        CompanLoggerController::get()->addBinder(std::move(monitor));
    }

    void createBinders()
    {
        std::set<std::pair<uint32_t, std::string>> enumerators;

        for (auto& iter : COMPAN::REF::getLogLevelBiMap().left)
            enumerators.insert(std::make_pair(static_cast<uint32_t>(iter.first), iter.second));

        // Walks the registered list of CompanLogger elements (sources)
        for (auto& source : CompanLoggerController::get()->sources()) {

            // The CompanLogger is registered as follows:
            // logger.[appname].[CompanLogger::name()]
            //
            // The CompanLogger::name can be in dotted notation.
            // The key which is added to the container is as:
            //  [appname].[CompanLogger::name()]

            ValueId monitorKey(appLoggerKey_, source->name());

            CompanEdgeProtocol::Value logValue;
            valueInit(logValue, enumerators);

            logValue.set_id(monitorKey);

            VariantValue::Ptr loggerPtr =
                    ws_.add(std::make_shared<VariantEnumValue>(ws_.getStrand(), logValue, VariantValue::Remote));
            if (!loggerPtr) { continue; }

            // helpful to set to 1 to turn on tracing through out
            if (0) { source->logLevel(LogLevel::Trace); }
            // sets the VariantValue to match the CompanLogger's current log level
            std::stringstream logLevelStr;
            logLevelStr << source->logLevel();
            loggerPtr->set(logLevelStr.str());

            bindSource(loggerPtr, source);
        }
    }

    void doRegister()
    {
        if (isMicroService_) {
            registerLogger();
            return;
        }

        CompanEdgeProtocol::Value appContainer;
        valueInit(appContainer, CompanEdgeProtocol::Container);
        appContainer.set_id(appLoggerKey_);

        ws_.set(appContainer, VariantValue::Remote);

        createBinders();
    }

private:
    std::string appName_;
    VariantValueStore& ws_;
    bool const isMicroService_;
    ValueId appLoggerKey_;

    std::map<ValueId, CompanLogger*> loggerIds_;

    SignalScopedConnection onAddToLoggerMap_;
    SignalScopedConnection onAddToLoggerAppMap_;

    VariantMapValue::Ptr loggerMap_;
    VariantMapValue::Ptr loggerAppMap_;
};

} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

ValueId const LoggerFactory::LoggerId("logger");

void LoggerFactory::createBinders(std::string const& appName, VariantValueStore& ws, bool const isMicroService)
{
    LoggerFactoryAutoRegister::Ptr autoRegister(
            std::make_shared<LoggerFactoryAutoRegister>(appName, ws, isMicroService));

    autoRegister->doRegister();
}
