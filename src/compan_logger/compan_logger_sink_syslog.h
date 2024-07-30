/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_sink_syslog.h
 @brief CompanLogger output sink to linux syslog
 */
#ifndef __Compan_LOGGER_SINK_SYSLOG_H__
#define __Compan_LOGGER_SINK_SYSLOG_H__

#include "Compan_logger_sink.h"

namespace Compan{
namespace Edge {

/*!
 * @brief Logger Sink to linux syslog
 */
class CompanLoggerSinkSyslog : public CompanLoggerSink {
public:
    CompanLoggerSinkSyslog(bool indent = true, int indentSize = 2);
    virtual ~CompanLoggerSinkSyslog() = default;

    virtual void operator()(CompanLogger const& logger, std::string const& msg, LogLevel level) override;
};

} // namespace Edge
} // namespace Compan

#endif // __Compan_LOGGER_SINK_SYSLOG_H__
