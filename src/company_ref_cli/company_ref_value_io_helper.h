/**
  Copyright © 2024 COMPAN REF
  @file company_ref_value_io_helper.h
  @brief CompanEdge - Value input output helper
*/

#ifndef __company_ref_VALUE_IO_HELPER_H__
#define __company_ref_VALUE_IO_HELPER_H__

// needed to enable boost::posix_time::nanoseconds in posix_time.hpp
#define BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Compan{
namespace Edge {

struct ValueIOHelper {
    static timespec timeFromIsoExtendedStrToTimeSpec(const std::string& isoExtenderStr);
    static timeval timeFromIsoExtendedStrToTimeVal(const std::string& isoExtenderStr);

    static std::string toIsoExtendedStr(const time_t timeSeconds);
    static std::string toIsoExtendedStr(const boost::posix_time::ptime pt);
    static std::string toIsoExtendedStr(const struct timespec& ts);
    static std::string toIsoExtendedStr(const struct timeval& tv);

    static std::string toOffsetTZ(const std::string posixTZ);

    static boost::posix_time::ptime toPtime(const struct timespec& ts);
    static boost::posix_time::ptime toPtime(const struct timeval& tv);

    static std::string toValueInputStr(const struct timespec ts);
    static std::string toValueInputStr(const struct timeval tv);
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VALUE_IO_HELPER_H__