/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_sink_syslog.cpp
 @brief CompanLogger output sink to linux syslog
 */

#include "Compan_logger_sink_syslog.h"
#include "Compan_logger.h"

#include <sstream>

#include <syslog.h>

using namespace Compan::Edge;

CompanLoggerSinkSyslog::CompanLoggerSinkSyslog(bool indent, int indentSize)
    : CompanLoggerSink(indent, indentSize)
{
}

void CompanLoggerSinkSyslog::operator()(CompanLogger const& logger, std::string const& msg, LogLevel level)
{
    std::ostringstream line;

    line << logger.name() << ": " << msg << std::endl;

    int priority(LOG_DEBUG);
    switch (level) {
    case LogLevel::Error: priority = LOG_ERR; break;
    case LogLevel::Warning: priority = LOG_WARNING; break;
    case LogLevel::Information: priority = LOG_INFO; break;
    case LogLevel::Debug:
    case LogLevel::None:
    case LogLevel::State:
    case LogLevel::Trace: break;
    }

    syslog(priority | LOG_USER, "%s", line.str().c_str());
}
