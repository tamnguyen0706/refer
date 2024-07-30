/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_sink_buffered.cpp
 @brief CompanLogger output sink to buffer
 */

#include "Compan_logger_sink_buffered.h"
#include "Compan_logger.h"

using namespace Compan::Edge;

CompanLoggerSinkBuffered::CompanLoggerSinkBuffered()
    : CompanLoggerSink(false, 0)
{
}
void CompanLoggerSinkBuffered::operator()(CompanLogger const& logger, std::string const& msg, LogLevel level)
{
    tmpStream_ << std::left << logger.name() << ": " << level << ": " << indent() << msg << std::endl;
}

bool CompanLoggerSinkBuffered::empty() const
{
    return tmpStream_.str().empty();
}

std::string CompanLoggerSinkBuffered::pop()
{
    std::string str(tmpStream_.str());

    std::string clearStrm;
    tmpStream_.str(clearStrm);

    return str;
}
