/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_sink.h
 @brief CompanLogger output sink
 */
#ifndef __Compan_LOGGER_SINK_H__
#define __Compan_LOGGER_SINK_H__

#include "Compan_logger_loglevel.h"

#include <string>

namespace Compan{
namespace Edge {

class CompanLogger;

class CompanLoggerSink {
public:
    CompanLoggerSink();
    CompanLoggerSink(bool indent, int indentSize);
    virtual ~CompanLoggerSink();
    virtual void operator()(CompanLogger const& logger, std::string const& msg, LogLevel level) = 0;
    void setIndent(bool b);
    bool isIndent() const;

protected:
    std::string const indent() const;

private:
    bool indent_;
    int const indentSize_;
    static const std::string nonIndentStr_;
};

inline void CompanLoggerSink::setIndent(bool b)
{
    indent_ = b;
}

inline bool CompanLoggerSink::isIndent() const
{
    return indent_;
}

} // namespace Edge
} // namespace Compan

#endif // __Compan_LOGGER_SINK_H__
