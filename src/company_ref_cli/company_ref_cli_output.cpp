/**
  Copyright © 2023 COMPAN REF
  @file company_ref_cli_output.cpp
  @brief cli output
*/

#include "company_ref_cli_output.h"
#include "company_ref_value_io_helper.h"

#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <company_ref_utils/company_ref_string_utils.h>
#include <Compan_logger/Compan_logger.h>

#include <sstream>

using namespace Compan::Edge;

namespace Compan{
namespace Edge {
CompanLogger CompanEdgeCliOutputLog("cli.output", LogLevel::Information);
}
} // namespace Compan

std::string const CompanEdgeCliOutput::ValueChangedTypeName = "ValueChanged";
std::string const CompanEdgeCliOutput::ValueRemovedTypeName = "ValueRemoved";
std::string const CompanEdgeCliOutput::VsResultTypeName = "VsResult";
std::string const CompanEdgeCliOutput::ErrorHeader = "Error";

CompanEdgeCliOutput::CompanEdgeCliOutput()
    : msgDelimiter_()
    , valuesDelimiter_()
    , valueIdValueDelimiter_()
    , timeZoneRule_("")
    , valueCategoryShown_(false)
    , valueOnly_(false)
    , format_(FORMAT_Compan)
{
    setDefaultAttribute();
}

void CompanEdgeCliOutput::setAttribute(PrintAttribute attrib, std::string&& valueStr)
{
    switch (attrib) {
    case PRINT_VALUES_DELIMITER: valuesDelimiter_ = valueStr; break;
    case PRINT_VALUEID_VALUE_SEPARATOR: valueIdValueDelimiter_ = valueStr; break;
    case PRINT_VALUE_CATEGORY_SHOWN: {
        bool boolVal(false);
        std::istringstream(valueStr) >> std::boolalpha >> boolVal;
        valueCategoryShown_ = boolVal;
    } break;
    case PRINT_VALUE_ONLY: {
        bool boolVal(false);
        std::istringstream(valueStr) >> std::boolalpha >> boolVal;
        valueOnly_ = boolVal;
    } break;
    case PRINT_TIMEZONE: timeZoneRule_ = valueStr; break;
    default: break;
    }
}

void CompanEdgeCliOutput::setFormatOption(FormatOption format)
{
    format_ = format;
}

void CompanEdgeCliOutput::setDefaultAttribute()
{
    msgDelimiter_ = "|";
    valuesDelimiter_ = "\n";
    valueIdValueDelimiter_ = ":";
    valueCategoryShown_ = true;
    valueOnly_ = false;
    timeZoneRule_ = "";
    format_ = FORMAT_Compan;
}

void CompanEdgeCliOutput::printError(std::string const& description)
{
    std::cout << ErrorHeader << msgDelimiter_ << description << std::endl;
}

void CompanEdgeCliOutput::print(google::protobuf::RepeatedPtrField<::CompanEdgeProtocol::Value> const& values)
{
    for (auto const& val : values) {
        // Purposely using std::cout, not using the logger because of the logger extra output
        std::cout << val.id() << valueIdValueDelimiter_ << toString(val) << valuesDelimiter_;
    }
}

void CompanEdgeCliOutput::printCsv(google::protobuf::RepeatedPtrField<::CompanEdgeProtocol::Value> const& values)
{
    std::ostringstream os;
    bool useDelimiter(false);
    // print in one line csv format
    for (auto const& val : values) {
        if (useDelimiter) {
            os << ",";
        } else {
            useDelimiter = true;
        }
        os << val.id() << valueIdValueDelimiter_ << toString(val);
    }
    std::cout << os.str() << valuesDelimiter_;
}

void CompanEdgeCliOutput::print(CompanEdgeProtocol::VsResult const& res)
{
    if (valueOnly_) {
        print(res.values());
        return;
    }

    if (valueCategoryShown_) {
        InfoLog(CompanEdgeCliOutputLog) << VsResultTypeName << valueIdValueDelimiter_
                                      << CompanEdgeProtocol::VsResult::Status_Name(res.status()) << valuesDelimiter_;
        InfoLog(CompanEdgeCliOutputLog) << VsResultTypeName << valueIdValueDelimiter_ << res.values().size()
                                      << valuesDelimiter_;
    }
    for (auto const& val : res.values()) {
        InfoLog(CompanEdgeCliOutputLog) << val.id() << valueIdValueDelimiter_ << toString(val) << valuesDelimiter_;
    }
}

void CompanEdgeCliOutput::printValue(CompanEdgeProtocol::VsResult const& res)
{
    if (valueOnly_) {
        for (auto const& val : res.values()) {
            // Purposely using std::cout, not using the logger because of the logger extra output
            std::cout << toString(val) << valuesDelimiter_;
        }
        return;
    }

    print(res);
}

void CompanEdgeCliOutput::print(CompanEdgeProtocol::VsMultiGetResult const& res)
{
    if (valueOnly_) {
        for (auto& result : res.results()) {
            if (result.error() != CompanEdgeProtocol::VsMultiGetResult::Success) { continue; }
            print(result.values());
        }
        return;
    }

    for (auto& result : res.results()) {
        if (result.error() != CompanEdgeProtocol::VsMultiGetResult::Success) { continue; }
        if (valueCategoryShown_) {
            InfoLog(CompanEdgeCliOutputLog)
                    << VsResultTypeName << valueIdValueDelimiter_ << result.values().size() << valuesDelimiter_;
        }
        for (auto const& val : result.values()) {
            InfoLog(CompanEdgeCliOutputLog) << val.id() << valueIdValueDelimiter_ << toString(val) << valuesDelimiter_;
        }
    }
}

void CompanEdgeCliOutput::print(CompanEdgeProtocol::VsMultiSetResult const& res)
{
    if (valueOnly_) {
        for (auto& result : res.results()) {
            if (result.error() != CompanEdgeProtocol::VsMultiSetResult::Success) { continue; }
            // Purposely using std::cout, not using the logger because of the logger extra output
            std::cout << result.id() << valuesDelimiter_;
        }
        return;
    }

    for (auto& result : res.results()) {
        if (result.error() != CompanEdgeProtocol::VsMultiSetResult::Success) { continue; }
        InfoLog(CompanEdgeCliOutputLog) << result.id() << valuesDelimiter_;
    }
}

void CompanEdgeCliOutput::print(CompanEdgeProtocol::ValueChanged const& res)
{
    if (valueOnly_) {
        print(res.value());
        return;
    }

    if (valueCategoryShown_) {
        InfoLog(CompanEdgeCliOutputLog) << ValueChangedTypeName << valueIdValueDelimiter_ << res.value().size()
                                      << valuesDelimiter_;
    }
    for (auto const& val : res.value()) {
        InfoLog(CompanEdgeCliOutputLog) << val.id() << valueIdValueDelimiter_ << toString(val) << valuesDelimiter_;
    }
}

void CompanEdgeCliOutput::print(CompanEdgeProtocol::ValueRemoved const& res)
{
    if (valueOnly_) {
        // Purposely using std::cout, not using the logger because of the logger extra output
        for (auto const& id : res.id()) { std::cout << id << valuesDelimiter_; }
        return;
    }

    if (valueCategoryShown_) {
        InfoLog(CompanEdgeCliOutputLog) << ValueRemovedTypeName << valueIdValueDelimiter_ << res.id().size()
                                      << valuesDelimiter_;
    }
    for (auto const& id : res.id()) { InfoLog(CompanEdgeCliOutputLog) << id << valuesDelimiter_; }
}

void CompanEdgeCliOutput::print(CompanEdgeProtocol::ClientMessage const& msg)
{
    if (msg.has_valuechanged()) { print(msg.valuechanged()); }
    if (msg.has_valueremoved()) { print(msg.valueremoved()); }
    if (msg.has_vsresult()) { print(msg.vsresult()); }
    if (msg.has_vssynccompleted() && valueCategoryShown_) {
        InfoLog(CompanEdgeCliOutputLog) << "SyncCompleted" << valueIdValueDelimiter_ << "0" << valuesDelimiter_;
    }
}

std::string CompanEdgeCliOutput::toString(CompanEdgeProtocol::Value const& val)
{
    if (FORMAT_TR069 != format_) { return to_string(val); }

    std::string v("");
    CompanEdgeProtocol::Value_Type const valueType = val.type();

    switch (valueType) {
    case CompanEdgeProtocol::Value_Type::TimeVal: {
        v = ValueIOHelper::toIsoExtendedStr(valueGet<timeval>(val));
        if (!timeZoneRule_.empty()) { v.append(ValueIOHelper::toOffsetTZ(timeZoneRule_)); }
    } break;
    case CompanEdgeProtocol::Value_Type::TimeSpec: {
        v = ValueIOHelper::toIsoExtendedStr(valueGet<timespec>(val));
        if (!timeZoneRule_.empty()) { v.append(ValueIOHelper::toOffsetTZ(timeZoneRule_)); }
    } break;
    case CompanEdgeProtocol::Value_Type::Set:
    case CompanEdgeProtocol::Value_Type::Vector:
    case CompanEdgeProtocol::Value_Type::UnorderedSet: {
        v = StringUtils::removeCurlyBrackets(to_string(val));
    } break;
    case CompanEdgeProtocol::Value_Type::Unset:
    case CompanEdgeProtocol::Value_Type::Unknown:
    case CompanEdgeProtocol::Value_Type::Bool:
    case CompanEdgeProtocol::Value_Type::Text:
    case CompanEdgeProtocol::Value_Type::Interval:
    case CompanEdgeProtocol::Value_Type::Enum:
    case CompanEdgeProtocol::Value_Type::UInterval:
    case CompanEdgeProtocol::Value_Type::ULLInterval:
    case CompanEdgeProtocol::Value_Type::Udid:
    case CompanEdgeProtocol::Value_Type::LLInterval:
    case CompanEdgeProtocol::Value_Type::SInterval:
    case CompanEdgeProtocol::Value_Type::USInterval:
    case CompanEdgeProtocol::Value_Type::IPv4:
    case CompanEdgeProtocol::Value_Type::EUI48:
    case CompanEdgeProtocol::Value_Type::IPv6:
    case CompanEdgeProtocol::Value_Type::Multi:
    case CompanEdgeProtocol::Value_Type::DInterval:
    case CompanEdgeProtocol::Value_Type::Container:
    case CompanEdgeProtocol::Value_Type::Struct:
    case CompanEdgeProtocol::Value_Type::ContainerAddTo:
    case CompanEdgeProtocol::Value_Type::ContainerRemoveFrom:
    case CompanEdgeProtocol::Value_Type_INT_MIN_SENTINEL_DO_NOT_USE_:
    case CompanEdgeProtocol::Value_Type_INT_MAX_SENTINEL_DO_NOT_USE_:
    default: {
        v = to_string(val);
    } break;
    }
    return v;
}

void CompanEdgeCliVerboseOutput::print(CompanEdgeProtocol::VsResult const& res)
{
    InfoLog(CompanEdgeCliOutputLog) << res;
}

void CompanEdgeCliVerboseOutput::printValue(CompanEdgeProtocol::VsResult const& res)
{
    InfoLog(CompanEdgeCliOutputLog) << res;
}

void CompanEdgeCliVerboseOutput::print(CompanEdgeProtocol::VsMultiGetResult const& res)
{
    InfoLog(CompanEdgeCliOutputLog) << res;
}

void CompanEdgeCliVerboseOutput::print(CompanEdgeProtocol::VsMultiSetResult const& res)
{
    InfoLog(CompanEdgeCliOutputLog) << res;
}

void CompanEdgeCliVerboseOutput::print(CompanEdgeProtocol::ValueChanged const& res)
{
    InfoLog(CompanEdgeCliOutputLog) << res;
}

void CompanEdgeCliVerboseOutput::print(CompanEdgeProtocol::ValueRemoved const& res)
{
    InfoLog(CompanEdgeCliOutputLog) << res;
}

void CompanEdgeCliVerboseOutput::print(CompanEdgeProtocol::ClientMessage const& msg)
{
    InfoLog(CompanEdgeCliOutputLog) << boost::posix_time::to_simple_string(
            boost::posix_time::ptime(boost::posix_time::second_clock::universal_time()))
                                  << " - size: " << msg.ByteSizeLong() << msg;

    if (msg.has_valuechanged()) { InfoLog(CompanEdgeCliOutputLog) << msg.valuechanged() << std::endl; }
    if (msg.has_valueremoved()) { InfoLog(CompanEdgeCliOutputLog) << msg.valueremoved() << std::endl; }
    if (msg.has_vsresult()) { InfoLog(CompanEdgeCliOutputLog) << msg.vsresult() << std::endl; }
    if (msg.has_vssynccompleted()) { InfoLog(CompanEdgeCliOutputLog) << "VS SYNCRONIZATION completed!" << std::endl; }
    InfoLog(CompanEdgeCliOutputLog) << std::endl;
}
