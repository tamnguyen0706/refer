/**
 Copyright © 2023 COMPAN REF
 @file Compan_logger_loglevel.h
@brief LogLevel enum and utils
 */
#ifndef __Compan_LOGGER_LOGLEVEL_H__
#define __Compan_LOGGER_LOGLEVEL_H__

#include <istream>
#include <ostream>

#include <boost/bimap.hpp>

namespace Compan{
namespace Edge {

enum class LogLevel { None, Error, Warning, Information, State, Debug, Trace, First = Error, Last = Trace };

typedef boost::bimap<LogLevel, std::string> LogLevelStrBiMap;

LogLevelStrBiMap const& getLogLevelBiMap();

std::istream& operator>>(std::istream& in, LogLevel& level);
std::ostream& operator<<(std::ostream& out, LogLevel const& level);

} // namespace Edge
} // namespace Compan

#endif // __Compan_LOGGER_LOGLEVEL_H__
