/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_dispatcher.h
 @brief Log Message Sink Dispatcher
 */
#ifndef __Compan_LOG_DISPATCHER_H__
#define __Compan_LOG_DISPATCHER_H__

#include "Compan_logger.h"
#include "Compan_logger_loglevel.h"

#include <deque>
#include <memory>
#include <mutex>

namespace Compan{
namespace Edge {

/*!
 * @brief Dispatches log messages to the Controller's registered LogSinks
 */
class CompanLoggerDispatcher {
public:
    using Ptr = std::unique_ptr<CompanLoggerDispatcher>;

    struct LogMessage {
        CompanLogger const& logger;
        std::string const msg;
        LogLevel const level;
    };

    CompanLoggerDispatcher();
    virtual ~CompanLoggerDispatcher() = default;

    /// Pushes a LogMessage into the dispatch queue
    virtual void push(LogMessage&& msg);

    /// Dispatches messages to the sinks
    void dispatchToSinks(LogMessage const& msg);

protected:
    std::mutex mutex_;
};

} // namespace Edge
} // namespace Compan

#endif // __Compan_LOG_DISPATCHER_H__
