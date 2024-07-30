/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_stream.h
 @brief Logger Capture Stream
 */
#ifndef __Compan_LOGGER_STREAM_H__
#define __Compan_LOGGER_STREAM_H__

#include "Compan_logger_loglevel.h"

#include <sstream>

namespace Compan{
namespace Edge {

class CompanLogger;

/*!
 * @brief CompanLogger Stream
 *
 * Captures the output stream to forward to a logger sink
 */
class CompanLoggerStream {
public:
    CompanLoggerStream(CompanLogger const& logger, LogLevel level);
    virtual ~CompanLoggerStream();

    CompanLoggerStream(CompanLoggerStream const& log) = delete;
    CompanLoggerStream& operator=(CompanLoggerStream const& log) = delete;

    std::ostream& log();

private:
    CompanLogger const& logger_;
    LogLevel const level_;
    std::ostringstream stream_;
};

} // namespace Edge
} // namespace Compan

#endif // __Compan_LOGGER_STREAM_H__
