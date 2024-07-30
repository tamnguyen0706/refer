/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_udid_value.cpp
 @brief Derived VariantValue for UDID
 */
#include "company_ref_variant_udid_value.h"

#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>

using namespace Compan::Edge;

VariantUdidValue::VariantUdidValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantValue(ctx, valueId, CompanEdgeProtocol::Unset)
{
}

VariantUdidValue::VariantUdidValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantValue(ctx, value, updateType)
{
}

uint64_t VariantUdidValue::get()
{
    return valueGet<uint64_t>(VariantValue::get());
}

ValueDataSet::Results VariantUdidValue::set(uint64_t const arg)
{
    CompanEdgeProtocol::Value value;
    valueInit(value, CompanEdgeProtocol::Udid);
    valueSet<uint64_t>(value, arg);

    return VariantValue::set(value);
}
