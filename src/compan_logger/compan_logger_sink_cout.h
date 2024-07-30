/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_sink_cout.h
 @brief CompanLogger output sink to std::cout
 */
#ifndef __LOGGER_Compan_LOGGER_SINK_COUT_H__
#define __LOGGER_Compan_LOGGER_SINK_COUT_H__

#include "Compan_logger_sink.h"

namespace Compan{
namespace Edge {

/*!
 * @brief Logger Sink to std::cout
 */
class CompanLoggerSinkCout : public CompanLoggerSink {
public:
    CompanLoggerSinkCout(bool indent = true, int indentSize = 2);
    virtual ~CompanLoggerSinkCout() = default;

    virtual void operator()(CompanLogger const& logger, std::string const& msg, LogLevel level) override;
};

} // namespace Edge
} // namespace Compan

#endif // __LOGGER_Compan_LOGGER_SINK_COUT_H__
