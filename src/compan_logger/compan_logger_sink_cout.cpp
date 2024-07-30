/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_sink_cout.cpp
 @brief CompanLogger output sink to std::cout
 */
#include "Compan_logger_sink_cout.h"
#include "Compan_logger.h"
#include "Compan_logger_format_helper.h"

#include <iomanip>
#include <iostream>

using namespace Compan::Edge;

namespace {
struct ShortHandLogLevel {
    LogLevel level;
};

std::ostream& operator<<(std::ostream& os, const ShortHandLogLevel& shll)
{
    switch (shll.level) {
    case LogLevel::None: break;
    case LogLevel::Error: os << "Err "; break;
    case LogLevel::Warning: os << "Warn"; break;
    case LogLevel::Information: os << "Info"; break;
    case LogLevel::State: os << "Stat"; break;
    case LogLevel::Debug: os << "Dbg "; break;
    case LogLevel::Trace: os << "Trc "; break;
    }
    return os;
}
} // namespace

CompanLoggerSinkCout::CompanLoggerSinkCout(bool indent, int indentSize)
    : CompanLoggerSink(indent, indentSize)
{
}

void CompanLoggerSinkCout::operator()(CompanLogger const& logger, std::string const& msg, LogLevel level)
{
    std::cout << std::setw(CompanLoggerFormatHelper::get()->maxLoggerNameLength()) << std::left << logger.name() << ": "
              << ShortHandLogLevel{level} << ": " << indent() << msg << std::endl;
}
