/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_valuedata.cpp
 @brief CompanEdgeProtocol::Value's data encapsulation
 */
#include "company_ref_variant_valuestore_valuedata.h"

#include <company_ref_protocol_utils/company_ref_pb_containers.h>
#include <company_ref_protocol_utils/company_ref_pb_enum.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_pb_operators.h>

#include <sstream>

using namespace Compan::Edge;

std::string ValueDataSet::resultStr(ValueDataSet::Results const arg)
{
    switch (arg) {
    case Success: return "Success";
    case SameValue: return "SameValue";
    case InvalidType: return "InvalidType";
    case RangeError: return "RangeError";
    case EnumError: return "EnumError";
    case AccessError: return "AccessError";
    }

    std::ostringstream strm;
    strm << "Unknown VariantValue::SetResults (" << static_cast<int>(arg) << ")";
    return strm.str();
}

namespace {

// range assignment
template <typename T>
typename std::enable_if<
        std::is_same<CompanValueTypes::IntervalValue, T>::value || std::is_same<CompanValueTypes::UIntervalValue, T>::value
                || std::is_same<CompanValueTypes::SIntervalValue, T>::value
                || std::is_same<CompanValueTypes::USIntervalValue, T>::value
                || std::is_same<CompanValueTypes::ULLIntervalValue, T>::value
                || std::is_same<CompanValueTypes::LLIntervalValue, T>::value
                || std::is_same<CompanValueTypes::DIntervalValue, T>::value,
        ValueDataSet::Results>::type
        assignTo(T& lhs, T const& rhs)
{
    if (lhs == rhs) return ValueDataSet::SameValue;

    if (lhs.min() || lhs.max()) {
        if (rhs.value() < lhs.min() || rhs.value() > lhs.max()) return ValueDataSet::RangeError;
    }

    lhs.set_value(rhs.value());

    return ValueDataSet::Success;
}

// simple value assignment
template <typename T>
typename std::enable_if<
        std::is_same<CompanValueTypes::BoolValue, T>::value || std::is_same<CompanValueTypes::TextValue, T>::value
                || std::is_same<CompanValueTypes::UdidValue, T>::value
                || std::is_same<CompanValueTypes::IPv4Value, T>::value
                || std::is_same<CompanValueTypes::IPv6Value, T>::value
                || std::is_same<CompanValueTypes::EUI48Value, T>::value
                || std::is_same<CompanValueTypes::MultiValue, T>::value
                || std::is_same<CompanValueTypes::TimeValValue, T>::value
                || std::is_same<CompanValueTypes::TimeSpecValue, T>::value,
        ValueDataSet::Results>::type
        assignTo(T& lhs, T const& rhs)
{
    if (lhs == rhs) return ValueDataSet::SameValue;

    lhs = rhs;
    return ValueDataSet::Success;
}

// Container assignments - using ValueContainer::constrain for data correctness
template <typename T>
typename std::enable_if<
        std::is_same<CompanValueTypes::MapValue, T>::value || std::is_same<CompanValueTypes::SetValue, T>::value
                || std::is_same<CompanValueTypes::UnorderedSetValue, T>::value
                || std::is_same<CompanValueTypes::VectorValue, T>::value,
        ValueDataSet::Results>::type
        assignTo(T& lhs, T const& rhs)
{
    if (lhs == rhs) return ValueDataSet::SameValue;

    lhs = rhs;
    ValueContainer::constrain(lhs);

    return ValueDataSet::Success;
}

// The odd duck
template <typename T>
typename std::enable_if<std::is_same<CompanValueTypes::EnumValue, T>::value, ValueDataSet::Results>::type assignTo(
        T& lhs,
        T const& rhs)
{
    if (lhs == rhs) return ValueDataSet::SameValue;

    if (!ValueEnumerators::checkRange(lhs, rhs.value())) return ValueDataSet::EnumError;

    lhs.set_value(rhs.value());

    return ValueDataSet::Success;
}

} // namespace

CompanEdgeProtocolValueData::CompanEdgeProtocolValueData(CompanEdgeProtocol::Value const& data)
    : value_(data)
{
    value_.clear_id();
}

CompanEdgeProtocolValueData::CompanEdgeProtocolValueData(
        CompanEdgeProtocol::Value_Type const type,
        CompanEdgeProtocol::Value_Access const access)
{
    valueInit(value_, type);
    value_.set_access(access);
    value_.clear_id();
}

ValueDataSet::Results CompanEdgeProtocolValueData::set(CompanEdgeProtocol::Value const& arg)
{
    // the stored value is not set
    if (value_.type() == CompanEdgeProtocol::Unset || value_.type() == CompanEdgeProtocol::Unknown)
        value_.set_type(arg.type());

    if (value_.type() != arg.type()) return ValueDataSet::InvalidType;

    ValueDataSet::Results retValue(ValueDataSet::InvalidType);

    switch (value_.type()) {

    case CompanEdgeProtocol::Bool: {
        retValue = assignTo(*value_.mutable_boolvalue(), arg.boolvalue());
    } break;

    case CompanEdgeProtocol::Text: {
        retValue = assignTo(*value_.mutable_textvalue(), arg.textvalue());
    } break;

    case CompanEdgeProtocol::Interval: {
        retValue = assignTo(*value_.mutable_intervalvalue(), arg.intervalvalue());
    } break;

    case CompanEdgeProtocol::UInterval: {
        retValue = assignTo(*value_.mutable_uintervalvalue(), arg.uintervalvalue());
    } break;

    case CompanEdgeProtocol::SInterval: {
        retValue = assignTo(*value_.mutable_sintervalvalue(), arg.sintervalvalue());
    } break;

    case CompanEdgeProtocol::USInterval: {
        retValue = assignTo(*value_.mutable_usintervalvalue(), arg.usintervalvalue());
    } break;

    case CompanEdgeProtocol::LLInterval: {
        retValue = assignTo(*value_.mutable_llintervalvalue(), arg.llintervalvalue());
    } break;

    case CompanEdgeProtocol::ULLInterval: {
        retValue = assignTo(*value_.mutable_ullintervalvalue(), arg.ullintervalvalue());
    } break;

    case CompanEdgeProtocol::DInterval: {
        retValue = assignTo(*value_.mutable_dintervalvalue(), arg.dintervalvalue());
    } break;

    case CompanEdgeProtocol::Udid: {
        retValue = assignTo(*value_.mutable_udidvalue(), arg.udidvalue());
    } break;

    case CompanEdgeProtocol::EUI48: {
        retValue = assignTo(*value_.mutable_eui48value(), arg.eui48value());
    } break;

    case CompanEdgeProtocol::Enum: {
        if (value_.enumvalue().enumerators_size() < arg.enumvalue().enumerators_size())
            *value_.mutable_enumvalue() = arg.enumvalue();

        retValue = assignTo(*value_.mutable_enumvalue(), arg.enumvalue());
    } break;

    case CompanEdgeProtocol::IPv4: {
        retValue = assignTo(*value_.mutable_ipv4value(), arg.ipv4value());
    } break;

    case CompanEdgeProtocol::IPv6: {
        retValue = assignTo(*value_.mutable_ipv6value(), arg.ipv6value());
    } break;

    case CompanEdgeProtocol::TimeVal: {
        retValue = assignTo(*value_.mutable_timevalvalue(), arg.timevalvalue());
    } break;

    case CompanEdgeProtocol::TimeSpec: {
        retValue = assignTo(*value_.mutable_timespecvalue(), arg.timespecvalue());
    } break;

    case CompanEdgeProtocol::Set: {
        retValue = assignTo(*value_.mutable_setvalue(), arg.setvalue());
    } break;

    case CompanEdgeProtocol::UnorderedSet: {
        retValue = assignTo(*value_.mutable_unorderedsetvalue(), arg.unorderedsetvalue());
    } break;

    case CompanEdgeProtocol::Vector: {
        retValue = assignTo(*value_.mutable_vectorvalue(), arg.vectorvalue());
    } break;

    case CompanEdgeProtocol::Container:
    case CompanEdgeProtocol::Struct: retValue = ValueDataSet::Success; break;

    case CompanEdgeProtocol::Unset:
    case CompanEdgeProtocol::Unknown:
    case CompanEdgeProtocol::Multi:
    case CompanEdgeProtocol::ContainerAddTo:
    case CompanEdgeProtocol::ContainerRemoveFrom:
    case CompanEdgeProtocol::Value_Type_INT_MIN_SENTINEL_DO_NOT_USE_:
    case CompanEdgeProtocol::Value_Type_INT_MAX_SENTINEL_DO_NOT_USE_: return ValueDataSet::InvalidType;
    }

    return retValue;
}

bool CompanEdgeProtocolValueData::hasData()
{
    if (value_.type() == CompanEdgeProtocol::Unset || value_.type() == CompanEdgeProtocol::Unknown) return false;

    switch (value_.type()) {

        // interval's will ALWAYS return true, since zero is a value
    case CompanEdgeProtocol::Bool:
    case CompanEdgeProtocol::Interval:
    case CompanEdgeProtocol::UInterval:
    case CompanEdgeProtocol::SInterval:
    case CompanEdgeProtocol::USInterval:
    case CompanEdgeProtocol::LLInterval:
    case CompanEdgeProtocol::ULLInterval:
    case CompanEdgeProtocol::DInterval:
    case CompanEdgeProtocol::Udid:
    case CompanEdgeProtocol::EUI48:
    case CompanEdgeProtocol::Enum:
    case CompanEdgeProtocol::TimeVal:
    case CompanEdgeProtocol::TimeSpec: return true;

    case CompanEdgeProtocol::Text: return !value_.textvalue().value().empty();

    case CompanEdgeProtocol::IPv4: return !value_.ipv4value().value().empty();

    case CompanEdgeProtocol::IPv6: return !value_.ipv6value().value().empty();

    case CompanEdgeProtocol::Set: return value_.setvalue().keys_size();
    case CompanEdgeProtocol::UnorderedSet: return value_.unorderedsetvalue().keys_size();
    case CompanEdgeProtocol::Vector: return value_.vectorvalue().keys_size();

    case CompanEdgeProtocol::Container:
    case CompanEdgeProtocol::Struct:
    case CompanEdgeProtocol::Unset:
    case CompanEdgeProtocol::Unknown:
    case CompanEdgeProtocol::Multi:
    case CompanEdgeProtocol::ContainerAddTo:
    case CompanEdgeProtocol::ContainerRemoveFrom:
    case CompanEdgeProtocol::Value_Type_INT_MIN_SENTINEL_DO_NOT_USE_:
    case CompanEdgeProtocol::Value_Type_INT_MAX_SENTINEL_DO_NOT_USE_: return false;
    }

    return false;
}
