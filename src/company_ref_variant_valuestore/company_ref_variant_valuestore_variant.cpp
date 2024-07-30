/**
  Copyright © 2023 COMPAN REF
  @file company_ref_variant_valuestore_variant.cpp
  @brief CompanEdge Variant Value Access Functions
*/

#include "company_ref_variant_valuestore_variant.h"

#include "company_ref_variant_valuestore_dispatcher.h"
#include "company_ref_variant_valuestore_valueid.h"

#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <company_ref_protocol_utils/company_ref_pb_containers.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_pb_operators.h>
#include <company_ref_protocol_utils/company_ref_pb_time.h>
#include <company_ref_protocol_utils/company_ref_pb_traits.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_utils/company_ref_regex_utils.h>
#include <company_ref_utils/company_ref_string_utils.h>
#include <Compan_logger/Compan_logger.h>

#include <company_ref_protocol/company_ref_protocol.pb.h>

#include <iomanip>
#include <iterator>
#include <regex>
#include <type_traits>

using namespace CompanEdgeProtocol;
using namespace Compan::Edge;

namespace {

template <typename T>
void parseContainerKeys(T& argValue, std::string const& arg)
{
    ValueContainer::clear(argValue);

    std::string str(COMPAN::REF::StringUtils::removeCurlyBrackets(arg));
    std::string::size_type startPos = 0;
    std::string::size_type nextPos = std::string::npos;

    while ((nextPos = str.find(',', startPos)) != std::string::npos) {
        std::string const key =
                std::regex_replace(str.substr(startPos, nextPos - startPos), RegexUtils::TrimPattern, "");

        ValueContainer::add(argValue, key);
        startPos = nextPos + 1;
    }

    std::string const key = std::regex_replace(str.substr(startPos, nextPos - startPos), RegexUtils::TrimPattern, "");

    ValueContainer::add(argValue, key);
}

} // namespace

CompanLogger VariantValueLog("variantvalue.data", LogLevel::Information);

VariantValue::~VariantValue() = default;

VariantValue::VariantValue(
        boost::asio::io_context::strand& ctx,
        ValueId const& valueId,
        CompanEdgeProtocol::Value_Type const type)
    : VariantValue(ctx, valueId, type, CompanEdgeProtocol::Value::ReadWrite)
{
}

VariantValue::VariantValue(
        boost::asio::io_context::strand& ctx,
        ValueId const& valueId,
        CompanEdgeProtocol::Value_Type const type,
        CompanEdgeProtocol::Value_Access const access)
    : ctx_(ctx)
    , signal_(ctx_)
    , value_(type, access)
    , valueId_(std::make_shared<ValueId>(valueId))
    , setUpdateType_(Local)
{
}

VariantValue::VariantValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& arg,
        SetUpdateType const updateType)
    : ctx_(ctx)
    , signal_(ctx_)
    , value_(arg)
    , valueId_(std::make_shared<ValueId>(arg.id()))
    , setUpdateType_(updateType)
{
}

VariantValue::VariantValue(
        boost::asio::io_context::strand& ctx,
        ValueId const& valueId,
        CompanEdgeProtocol::Value_Access const access,
        std::set<std::pair<uint32_t, std::string>> const& enumerator)
    : ctx_(ctx)
    , signal_(ctx_)
    , value_(CompanEdgeProtocol::Enum, access)
    , valueId_(std::make_shared<ValueId>(valueId))
    , setUpdateType_(Local)
{
    if (enumerator.empty()) return;

    CompanEdgeProtocol::Value enumVal;
    valueInit(enumVal, enumerator);

    value_.set(enumVal);
}

ValueDataSet::Results VariantValue::set(std::string const& arg, SetUpdateType const updateType)
{
    if (value_.access() == CompanEdgeProtocol::Value_Access_ReadOnly && updateType == Local)
        return ValueDataSet::AccessError;

    if (value_.type() == CompanEdgeProtocol::Unset || value_.type() == CompanEdgeProtocol::Unknown)
        return ValueDataSet::InvalidType;

    CompanEdgeProtocol::Value updateValue(get());

    switch (updateValue.type()) {
    case CompanEdgeProtocol::Bool: {
        if (arg.empty()) return ValueDataSet::InvalidType;
        bool argValue(false);
        std::istringstream(arg) >> std::boolalpha >> argValue;
        valueSet(updateValue, argValue);
    } break;

    case CompanEdgeProtocol::Text: valueSet(updateValue, arg); break;
    case CompanEdgeProtocol::Enum:
    case CompanEdgeProtocol::IPv4:
    case CompanEdgeProtocol::IPv6:
        if (arg.empty()) return ValueDataSet::InvalidType;
        valueSet(updateValue, arg);
        break;

    case CompanEdgeProtocol::EUI48: {
        if (arg.size() != 17u) // strlen("xx:xx:xx:xx:xx:xx")
            return ValueDataSet::InvalidType;
        std::istringstream strm(arg);
        uint64_t eui48(0);

        while (!strm.eof()) {
            char colon(0);
            uint16_t byte(0);
            strm >> std::hex >> byte >> std::dec >> colon;
            eui48 = (eui48 << 8) | byte;
        }

        valueSet(updateValue, static_cast<uint64_t>(eui48));
    } break;

    case CompanEdgeProtocol::Interval:
    case CompanEdgeProtocol::SInterval:
        if (!isdigit(arg[0]) && arg[0] != '-') return ValueDataSet::InvalidType;
        valueSet(updateValue, static_cast<int32_t>(std::stoll(arg)));
        break;

    case CompanEdgeProtocol::UInterval:
    case CompanEdgeProtocol::USInterval:
        if (!isdigit(arg[0])) return ValueDataSet::InvalidType;
        valueSet(updateValue, static_cast<int32_t>(std::stoull(arg)));
        break;

    case CompanEdgeProtocol::LLInterval:
        if (!isdigit(arg[0]) && arg[0] != '-') return ValueDataSet::InvalidType;
        valueSet(updateValue, static_cast<int64_t>(std::stoll(arg)));
        break;

    case CompanEdgeProtocol::Udid:
    case CompanEdgeProtocol::ULLInterval:
        if (!isdigit(arg[0])) return ValueDataSet::InvalidType;
        valueSet(updateValue, static_cast<uint64_t>(std::stoull(arg)));
        break;

    case CompanEdgeProtocol::DInterval:
        if (!isdigit(arg[0]) && arg[0] != '-') return ValueDataSet::InvalidType;
        valueSet(updateValue, std::stod(arg));
        break;

    case CompanEdgeProtocol::TimeVal: {
        if (!isdigit(arg[0])) return ValueDataSet::InvalidType;
        struct timeval argValue = {0ull, 0ull};
        ValueTime::fromString(argValue, arg);
        valueSet(updateValue, argValue);
    } break;

    case CompanEdgeProtocol::TimeSpec: {
        if (!isdigit(arg[0])) return ValueDataSet::InvalidType;
        struct timespec argValue = {0ull, 0ull};
        ValueTime::fromString(argValue, arg);
        valueSet(updateValue, argValue);
    } break;

    case CompanEdgeProtocol::Set: {
        parseContainerKeys(*updateValue.mutable_setvalue(), arg);
    } break;

    case CompanEdgeProtocol::UnorderedSet: {
        parseContainerKeys(*updateValue.mutable_unorderedsetvalue(), arg);
    } break;

    case CompanEdgeProtocol::Vector: {
        parseContainerKeys(*updateValue.mutable_vectorvalue(), arg);
    } break;

    case CompanEdgeProtocol::Container:
    case CompanEdgeProtocol::Struct:
        // no point in continuing, right?
        signal();
        return ValueDataSet::Success;
        break;

    case CompanEdgeProtocol::Unset:
    case CompanEdgeProtocol::Unknown:
    case CompanEdgeProtocol::Multi:

    case CompanEdgeProtocol::ContainerAddTo:
    case CompanEdgeProtocol::ContainerRemoveFrom:

    case CompanEdgeProtocol::Value_Type_INT_MIN_SENTINEL_DO_NOT_USE_:
    case CompanEdgeProtocol::Value_Type_INT_MAX_SENTINEL_DO_NOT_USE_: return ValueDataSet::InvalidType;
    }

    return set(updateValue, updateType);
}

ValueDataSet::Results VariantValue::set(CompanEdgeProtocol::Value const& arg, SetUpdateType const updateType)
{
    if (value_.access() == CompanEdgeProtocol::Value_Access_ReadOnly && updateType == Local)
        return ValueDataSet::AccessError;

    // the stored value is not set
    if (value_.type() == CompanEdgeProtocol::Unset || value_.type() == CompanEdgeProtocol::Unknown) value_.type(arg.type());

    if (value_.type() != arg.type()) return ValueDataSet::InvalidType;

    setUpdateType_ = updateType;

    ValueDataSet::Results retValue;
    if (dataDispatcher_) {
        VariantValueDispatcher::ProtocolValueSetPtr setTask(std::make_shared<VariantValueDispatcher::ProtocolValueSet>(
                std::bind(&CompanEdgeProtocolValueData::set, &value_, std::placeholders::_1)));

        std::future<ValueDataSet::Results> result = setTask->get_future();

        dataDispatcher_->setCompanEdgeProtocolValue(setTask, arg);

        retValue = result.get();
    } else
        retValue = value_.set(arg);

    if ((retValue == ValueDataSet::Success)
        && (value_.access() == Value_Access_WriteOnce || arg.access() == CompanEdgeProtocol::Value_Access_ReadOnly))
        value_.access(CompanEdgeProtocol::Value_Access_ReadOnly);

    if (retValue == ValueDataSet::Success) signal();

    return retValue;
}

bool VariantValue::hasData()
{
    if (dataDispatcher_) {
        VariantValueDispatcher::ProtocolValueHasDataPtr hasTask(
                std::make_shared<VariantValueDispatcher::ProtocolValueHasData>(
                        std::bind(&CompanEdgeProtocolValueData::hasData, &value_)));

        std::future<bool> result = hasTask->get_future();

        dataDispatcher_->hasDataCompanEdgeProtocolValue(hasTask);

        return result.get();
    }

    return value_.hasData();
}

std::string VariantValue::str() const
{
    return to_string(get());
}

ValueId const VariantValue::id() const
{
    if (valueId_)
        return *valueId_;
    else
        ErrorLog(VariantValueLog) << "Value Id is invalid:" << value_.hashToken() << std::endl;

    return ValueId();
}

CompanEdgeProtocol::Value VariantValue::get() const
{
    CompanEdgeProtocol::Value value;

    if (dataDispatcher_) {
        VariantValueDispatcher::ProtocolValueGetPtr getTask(std::make_shared<VariantValueDispatcher::ProtocolValueGet>(
                std::bind(&CompanEdgeProtocolValueData::get, &value_)));

        std::future<CompanEdgeProtocol::Value> result = getTask->get_future();
        dataDispatcher_->getCompanEdgeProtocolValue(getTask);

        value = result.get();
    } else
        value = value_.get();

    if (valueId_)
        value.set_id(*valueId_);
    else
        ErrorLog(VariantValueLog) << "Value Id is invalid:" << value_.hashToken() << std::endl;

    return value;
}

void VariantValue::addChild(VariantValue::Ptr valuePtr)
{
    children_.emplace(valuePtr->id().leaf(), valuePtr);
}

void VariantValue::delChild(ValueId const& valueId)
{
    ValueId childId(valueId.leaf());

    if (children_.count(childId) == 0) return;

    VariantValue::Ptr childPtr = children_.at(childId);
    if (childPtr) childPtr->disconnect();

    children_.erase(childId);
}

bool VariantValue::hasChildren() const
{
    return !children_.empty();
}

VariantValue::ChildMapType const VariantValue::getChildren() const
{
    return children_;
}

VariantValue::Ptr VariantValue::getChild(ValueId const childId) const
{
    if (children_.empty()) return nullptr;

    if (children_.count(childId)) return children_.at(childId);

    return nullptr;
}

VariantValue::Ptr VariantValue::getFirst() const
{
    if (children_.empty()) return nullptr;

    return children_.begin()->second;
}

void VariantValue::signal(VariantValue::Ptr valuePtr)
{
    if (wsChangeNotification_) wsChangeNotification_(valuePtr);

    signal_(valuePtr);
}

void VariantValue::disconnect()
{
    dataDispatcher_.reset();

    signal_.disconnectAll();

    parent_.reset();
}
