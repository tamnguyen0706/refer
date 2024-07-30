/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_sink_buffered.h
 @brief CompanLogger output sink to buffer
 */
#ifndef __Compan_LOGGER_SINK_BUFFERED_H__
#define __Compan_LOGGER_SINK_BUFFERED_H__

#include "Compan_logger_sink.h"

#include <sstream>

namespace Compan{
namespace Edge {

/*!
 * @brief Stores the log info in a buffer
 *
 * Primary use case is for unit testing
 */
class CompanLoggerSinkBuffered : public CompanLoggerSink {
public:
    CompanLoggerSinkBuffered();
    virtual ~CompanLoggerSinkBuffered() = default;

    virtual void operator()(CompanLogger const& logger, std::string const& msg, LogLevel level) override;

    bool empty() const;

    std::string pop();

private:
    std::stringstream tmpStream_;
};

} // namespace Edge
} // namespace Compan

#endif // __Compan_LOGGER_SINK_BUFFERED_H__
