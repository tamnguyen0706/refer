/**
  Copyright © 2024 COMPAN REF
  @file company_ref_vclio_helper.cpp
  @brief CompanEdge - command line input output helper
*/

#include "company_ref_value_io_helper.h"

//
// IsoExtendedStr format "yyyy-mm-ddThh:mm:ss.microseconds" ex: "1975-09-22T04:05:52.3850"
// In VariantValueStore:
// TimeSpecValue input/ouput format "seconds:nanoseconds"  ex: "180590752:385000000"
// TimeValValue input/output format "seconds:microseconds" ex: "180590752:385000"
//
// IsoExtendedStr ex: "1975-09-22T04:05:52.00385"
// In VariantValueStore:
// TimeSpecValue ex: "180590752:003850000"
// TimeValValue ex: "180590752:003850"
//
#include <boost/date_time.hpp>
#include <exception>
#include <iostream>
#include <regex>

using namespace Compan::Edge;

timespec ValueIOHelper::timeFromIsoExtendedStrToTimeSpec(const std::string& isoExtendedStr)
{
    // remove any time zone info
    std::regex datePattern("\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}(\\.\\d+)?");
    std::smatch match;

    if (!std::regex_search(isoExtendedStr, match, datePattern)) {
        throw std::runtime_error("Date input string invalid");
    }

    boost::posix_time::ptime pt(boost::posix_time::from_iso_extended_string(match.str()));
    boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
    boost::posix_time::time_duration diff = pt - epoch;

    timespec ts;
    ts.tv_sec = diff.total_seconds();
    ts.tv_nsec = diff.total_nanoseconds() % 1000000000;
    // another way to calculate
    // ts.tv_sec = static_cast<time_t>(diff.total_seconds());
    // ts.tv_nsec = static_cast<long>(diff.fractional_seconds());
    // std::cout << "ts:" << toValueInputStr(ts) << std::endl;

    return ts;
}

timeval ValueIOHelper::timeFromIsoExtendedStrToTimeVal(const std::string& isoExtendedStr)
{
    timespec ts(timeFromIsoExtendedStrToTimeSpec(isoExtendedStr));
    timeval tv;
    tv.tv_sec = ts.tv_sec;
    tv.tv_usec = ts.tv_nsec / 1000; // to microseconds
    // std::cout << "tv:" << toValueInputStr(tv) << std::endl;

    return tv;
}

std::string ValueIOHelper::toIsoExtendedStr(const time_t timeSeconds)
{
    struct tm* nowtm = gmtime(&timeSeconds);
    // Create a boost::posix_time::ptime object
    boost::posix_time::ptime pt(
            boost::gregorian::date(nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday),
            boost::posix_time::time_duration(nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec));

    return to_iso_extended_string(pt);
}

std::string ValueIOHelper::toIsoExtendedStr(const boost::posix_time::ptime pt)
{
    return to_iso_extended_string(pt);
}

std::string ValueIOHelper::toIsoExtendedStr(const struct timespec& ts)
{
    return to_iso_extended_string(toPtime(ts));
}

std::string ValueIOHelper::toIsoExtendedStr(const struct timeval& tv)
{
    return to_iso_extended_string(toPtime(tv));
}

std::string COMPAN::REF::ValueIOHelper::toOffsetTZ(const std::string posixTZ)
{
    std::stringstream ss;
    try {
        boost::local_time::time_zone_ptr tz(new boost::local_time::posix_time_zone(posixTZ));
        boost::posix_time::time_duration offset = tz->base_utc_offset();

        ss << (offset.is_negative() ? "-" : "+");
        ss << std::setw(2) << std::setfill('0') << std::abs(offset.hours()) << ":";
        ss << std::setw(2) << std::setfill('0') << offset.minutes();
    } catch (const std::exception& e) {
        return "";
    }

    return ss.str();
}

boost::posix_time::ptime ValueIOHelper::toPtime(const struct timespec& ts)
{
    boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
    boost::posix_time::time_duration duration(
            boost::posix_time::seconds(ts.tv_sec) + boost::posix_time::nanoseconds(ts.tv_nsec));
    return epoch + duration;
}

boost::posix_time::ptime ValueIOHelper::toPtime(const struct timeval& tv)
{
    boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
    boost::posix_time::time_duration duration(
            boost::posix_time::seconds(tv.tv_sec) + boost::posix_time::microseconds(tv.tv_usec));
    return epoch + duration;
}

std::string ValueIOHelper::toValueInputStr(const struct timespec ts)
{
    std::stringstream ss;
    boost::posix_time::time_duration duration(boost::posix_time::nanoseconds(ts.tv_nsec));
    ss << ts.tv_sec << ":" << std::setw(9) << std::setfill('0') << (duration.total_nanoseconds() % 1000000000);
    return ss.str();
}

std::string ValueIOHelper::toValueInputStr(const struct timeval tv)
{
    std::stringstream ss;
    boost::posix_time::time_duration duration(boost::posix_time::microseconds(tv.tv_usec));
    ss << tv.tv_sec << ":" << std::setw(6) << std::setfill('0') << (duration.total_microseconds() % 1000000);
    return ss.str();
}
