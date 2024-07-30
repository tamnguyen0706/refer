/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_timespec_value.cpp
 @brief Derived VariantValue for TimeSpec
 */
#include "company_ref_variant_timespec_value.h"

#include "company_ref_variant_valuestore_valueid.h"

#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_pb_traits.h>

using namespace Compan::Edge;

VariantTimeSpecValue::VariantTimeSpecValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantValue(ctx, valueId, CompanEdgeProtocol::Unset)
{
}

VariantTimeSpecValue::VariantTimeSpecValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantValue(ctx, value, updateType)
{
}

struct timespec VariantTimeSpecValue::get()
{
    return valueGet<struct timespec>(VariantValue::get());
}

ValueDataSet::Results VariantTimeSpecValue::set(struct timespec const arg)
{
    CompanEdgeProtocol::Value value;
    valueInit(value, CompanEdgeProtocol::TimeSpec);
    valueSet<struct timespec>(value, arg);

    return VariantValue::set(value);
}
