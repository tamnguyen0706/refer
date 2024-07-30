/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_dispatcher.cpp
 @brief Log Message Sink Dispatcher
 */

#include "Compan_logger_dispatcher.h"
#include "Compan_logger_controller.h"
#include "Compan_logger_sink.h"

using namespace Compan::Edge;

CompanLoggerDispatcher ::CompanLoggerDispatcher()
{
}

void CompanLoggerDispatcher::push(LogMessage&& msg)
{
    std::lock_guard<std::mutex> lock(mutex_);
    dispatchToSinks(msg);
}

void CompanLoggerDispatcher::dispatchToSinks(LogMessage const& msg)
{
    for (auto& sink : CompanLoggerController::get()->sinks()) (*sink)(msg.logger, msg.msg, msg.level);
}
