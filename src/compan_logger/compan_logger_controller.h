/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_controller.h
 @brief Main Logger Controller
 */
#ifndef __Compan_LOGGER_CONTROLLER_H__
#define __Compan_LOGGER_CONTROLLER_H__

#include "Compan_logger_loglevel.h"

#include <map>
#include <memory>
#include <vector>

namespace Compan{
namespace Edge {

class CompanLoggerSink;
class CompanLoggerDispatcher;
class CompanLogger;
class CompanLoggerBinder;

/*!
 * @brief CompanLogger Controller
 *
 * Manages Sources and Sinks for logging.
 *
 * This is a single instance object accessible via CompanLoggerController::get
 */
class CompanLoggerController {
public:
    using LoggerSinks = std::vector<CompanLoggerSink*>;
    using LoggerSources = std::vector<CompanLogger*>;
    using LoggerBinders = std::map<std::string, std::unique_ptr<CompanLoggerBinder>>;

    using Ptr = std::shared_ptr<CompanLoggerController>;

    virtual ~CompanLoggerController();

    /// Single instance access
    static CompanLoggerController::Ptr get();

    /// Allows for setting a alternate sink dispatch mechanism
    void setDispatcher(std::unique_ptr<CompanLoggerDispatcher>&& arg);

    /*!
     * Adds a sink to the logging engine
     * @param sink
     */
    void addSink(CompanLoggerSink* sink);

    /*!
     * Removes a sink from the logging engine
     * @param sink
     */
    void removeSink(CompanLoggerSink* sink);

    /// Returns the current sinks
    LoggerSinks const& sinks() const;

    /*!
     * Adds a logger source to the logging engine
     */
    void addLogger(CompanLogger& logger);

    /*!
     * Removes a logger source from the logging engine
     */
    void removeLogger(CompanLogger& logger);

    /// Returns the current sources
    LoggerSources const& sources() const;

    /*!
     * Adds a logger bind object to the logging engine
     */
    bool addBinder(std::unique_ptr<CompanLoggerBinder>&& bindObject);

    /*!
     * Removes a logger bind object to the logging engine
     */
    void removeBinder(std::string const& name);

    /// clears the logger bind objects from the logging engine - useful for unittests
    void clearBinders();

    /*!
     * Sends log messages to the dispatcher
     * @param logger
     * @param msg
     * @param level
     */
    void log(CompanLogger const& logger, LogLevel const level, std::string const& msg);

protected:
    CompanLoggerController();

private:
    std::unique_ptr<CompanLoggerDispatcher> dispatcher_;

    LoggerSinks sinks_;
    LoggerSources sources_;
    LoggerBinders binders_;
};

inline CompanLoggerController::LoggerSinks const& CompanLoggerController::sinks() const
{
    return sinks_;
}

inline CompanLoggerController::LoggerSources const& CompanLoggerController::sources() const
{
    return sources_;
}

} // namespace Edge
} // namespace Compan

#endif // __Compan_LOGGER_CONTROLLER_H__
