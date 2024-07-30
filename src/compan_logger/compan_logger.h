/**
  Copyright © 2023 COMPAN REF
  @file Compan_logger.h
  @brief CompanLogger
*/

#ifndef __Compan_LOGGER_H__
#define __Compan_LOGGER_H__

#include "Compan_logger_loglevel.h"
#include "Compan_logger_stream.h"

#include <iosfwd>
#include <iostream>
#include <memory>
#include <string>

#include <boost/signals2.hpp>

namespace Compan{
namespace Edge {

class CompanLogger {
public:
    using LogLevelSignal = boost::signals2::signal<void(LogLevel const)>;

    CompanLogger(std::string const& name, LogLevel initialLevel = LogLevel::Error);
    virtual ~CompanLogger();

    CompanLogger(CompanLogger const& loglogger) = delete;
    CompanLogger& operator=(CompanLogger const& loglogger) = delete;

    /// Checks if the current LogLevel matches the argument
    bool isEnabled(LogLevel const level) const;

    /// Sets the LogLevel and signals the listeners
    void logLevel(LogLevel const level);

    /// Returns the current LogLevel
    LogLevel logLevel() const;

    /// Loger's name
    std::string const& name() const;

    /// Connects a listener to LogLevel notifications
    boost::signals2::connection connectLogLevelListener(LogLevelSignal::slot_type const&);

private:
    std::string const name_;
    LogLevel currentLevel_;

    LogLevelSignal logLevelSignal_;
};

#define DoLog(logger, level) logger.isEnabled(level) && CompanLoggerStream(logger, level).log()
#define ErrorLog(logger) DoLog(logger, COMPAN::REF::LogLevel::Error)
#define WarnLog(logger) DoLog(logger, COMPAN::REF::LogLevel::Warning)
#define InfoLog(logger) DoLog(logger, COMPAN::REF::LogLevel::Information)
#define StateLog(logger) DoLog(logger, COMPAN::REF::LogLevel::State)
#define DebugLog(logger) DoLog(logger, COMPAN::REF::LogLevel::Debug)
#define TraceLog(logger) DoLog(logger, COMPAN::REF::LogLevel::Trace)

#define Compan_STR_CONCAT2(x, y) x##y
#define Compan_STR_CONCAT(x, y) Compan_STR_CONCAT2(x, y)
#define Compan_MK_UNIQUE_VAR Compan_STR_CONCAT(__uniq_var, __LINE__)

/**
 * \brief Prints enter/exit messages to the passed log logger
 */
#define FunctionLog(Logger) COMPAN::REF::_FunctionLog_ Compan_MK_UNIQUE_VAR(Logger, __PRETTY_FUNCTION__)
/**
 * \brief Prints enter/exit messages to the passed log logger but doesn't flush the log allowing you to send more log.
 * Should *only* be used in the top of a function.
 */
#define FunctionArgLog(Logger)                                                        \
    COMPAN::REF::_FunctionLog_ Compan_MK_UNIQUE_VAR(Logger, __PRETTY_FUNCTION__, false); \
    DoLog(Logger, _FunctionLog_::FunctionLogLevel()) << "Enter: " << __PRETTY_FUNCTION__ << (" ")

class _FunctionLog_ {
public:
    _FunctionLog_(CompanLogger& logger, char const* func, bool logEnter = true);
    ~_FunctionLog_();
    _FunctionLog_(_FunctionLog_ const&) = delete;
    _FunctionLog_& operator=(_FunctionLog_ const&) = delete;

public:
    static constexpr LogLevel FunctionLogLevel();

private:
    CompanLogger& logger_;
    char const* func_;
};

inline bool CompanLogger::isEnabled(LogLevel level) const
{
    return static_cast<int>(level) <= static_cast<int>(currentLevel_);
}

inline LogLevel CompanLogger::logLevel() const
{
    return currentLevel_;
}

inline std::string const& CompanLogger::name() const
{
    return name_;
}

inline boost::signals2::connection CompanLogger::connectLogLevelListener(LogLevelSignal::slot_type const& cb)
{
    return logLevelSignal_.connect(cb);
}

inline constexpr LogLevel _FunctionLog_::FunctionLogLevel()
{
    return LogLevel::Trace;
}

} // namespace Edge
} // namespace Compan

#endif /*__Compan_LOGGER_H__*/
