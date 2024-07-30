/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_binder.h
 @brief CompanLogger Bind class
 */
#ifndef __Compan_LOGGER_BINDER_H__
#define __Compan_LOGGER_BINDER_H__

#include "Compan_logger_loglevel.h"

#include <boost/signals2.hpp>

#include <memory>

namespace Compan{
namespace Edge {

class CompanLogger;

/*!
 * Allows an object to be bound for notifications for
 * LogLevel notifications and respond appropriately
 *
 */
class CompanLoggerBinder {
public:
    using Ptr = std::unique_ptr<CompanLoggerBinder>;

    CompanLoggerBinder(std::string const& name, CompanLogger& logger);
    virtual ~CompanLoggerBinder();

    std::string const& name() const;

    /// Connected signal slot from the CompanLogger for notifications
    virtual void onLogLevel(LogLevel const logLevel) = 0;

    LogLevel logLevel() const;
    void logLevel(LogLevel const arg);

private:
    std::string const name_;
    CompanLogger& logger_;

    boost::signals2::scoped_connection loggerConnection_;
};

inline std::string const& CompanLoggerBinder::name() const
{
    return name_;
}

} // namespace Edge
} // namespace Compan

#endif // __Compan_LOGGER_BINDER_H__
