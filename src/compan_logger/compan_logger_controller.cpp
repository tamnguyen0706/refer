/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_controller.cpp
 @brief Main Logger Controller
 */

#include "Compan_logger_controller.h"
#include "Compan_logger_binder.h"
#include "Compan_logger_dispatcher.h"
#include "Compan_logger_format_helper.h"
#include "Compan_logger_sink.h"

#include <algorithm>

using namespace Compan::Edge;

CompanLoggerController::CompanLoggerController()
    : dispatcher_(std::make_unique<CompanLoggerDispatcher>())
{
}

CompanLoggerController::~CompanLoggerController()
{
}

CompanLoggerController::Ptr CompanLoggerController::get()
{
    static CompanLoggerController::Ptr contoller = CompanLoggerController::Ptr(new CompanLoggerController);
    return contoller;
}

void CompanLoggerController::setDispatcher(CompanLoggerDispatcher::Ptr&& arg)
{
    dispatcher_ = std::move(arg);
}

void CompanLoggerController::addSink(CompanLoggerSink* sink)
{
    sinks_.push_back(sink);
}

void CompanLoggerController::removeSink(CompanLoggerSink* sink)
{
    auto it = find(sinks_.begin(), sinks_.end(), sink);
    if (it != sinks_.end()) { sinks_.erase(it); }
}

void CompanLoggerController::addLogger(CompanLogger& logger)
{
    CompanLoggerFormatHelper::get()->updateMaxLoggerNameLength(logger);
    sources_.push_back(&logger);
}

void CompanLoggerController::removeLogger(CompanLogger& logger)
{
    auto it = find(sources_.begin(), sources_.end(), &logger);
    if (it != sources_.end()) { sources_.erase(it); }

    CompanLoggerFormatHelper::get()->resetLoggerNameLength();
    for (auto it = sources_.begin(); it != sources_.end(); ++it) {
        CompanLoggerFormatHelper::get()->updateMaxLoggerNameLength(**it);
    }
}

void CompanLoggerController::log(CompanLogger const& logger, LogLevel const level, std::string const& msg)
{
    CompanLoggerDispatcher::LogMessage logMsg = {logger, msg, level};

    dispatcher_->push(std::move(logMsg));
}

bool CompanLoggerController::addBinder(std::unique_ptr<CompanLoggerBinder>&& bindObject)
{
    auto insert_iter = binders_.emplace(bindObject->name(), std::move(bindObject));
    return insert_iter.second;
}

void CompanLoggerController::removeBinder(std::string const& name)
{
    binders_.erase(name);
}

void CompanLoggerController::clearBinders()
{
    binders_ = LoggerBinders();
}
