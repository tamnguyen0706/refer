/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_loglevel.cpp
 @brief LogLevel enum and utils
 */

#include "Compan_logger_loglevel.h"

#include <boost/assign.hpp>

using namespace Compan::Edge;

static LogLevelStrBiMap logLevelStrings_ = boost::assign::list_of<LogLevelStrBiMap::relation>(LogLevel::None, "None")(
        LogLevel::Error,
        "Error")(LogLevel::Warning, "Warning")(LogLevel::Information, "Information")(LogLevel::State, "State")(
        LogLevel::Debug,
        "Debug")(LogLevel::Trace, "Trace");

boost::bimap<LogLevel, std::string> const& COMPAN::REF::getLogLevelBiMap()
{
    return logLevelStrings_;
}

std::istream& COMPAN::REF::operator>>(std::istream& in, LogLevel& level)
{
    std::string name;
    in >> name;

    auto iter = logLevelStrings_.right.find(name);
    if (iter == logLevelStrings_.right.end())
        level = LogLevel::None;
    else
        level = iter->second;

    return in;
}

std::ostream& COMPAN::REF::operator<<(std::ostream& os, LogLevel const& level)
{
    auto iter = logLevelStrings_.left.find(level);
    if (iter != logLevelStrings_.left.end()) return os << iter->second;

    return os;
}
