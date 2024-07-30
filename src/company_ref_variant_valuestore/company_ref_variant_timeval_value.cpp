/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_timeval_value.cpp
 @brief Derived VariantValue for TimeVal
 */
#include "company_ref_variant_timeval_value.h"

#include "company_ref_variant_valuestore_valueid.h"

#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_pb_traits.h>

using namespace Compan::Edge;

VariantTimeValValue::VariantTimeValValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantValue(ctx, valueId, CompanEdgeProtocol::Unset)
{
}

VariantTimeValValue::VariantTimeValValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantValue(ctx, value, updateType)
{
}

struct timeval VariantTimeValValue::get()
{
    return valueGet<struct timeval>(VariantValue::get());
}

ValueDataSet::Results VariantTimeValValue::set(struct timeval const arg)
{
    CompanEdgeProtocol::Value value;
    valueInit(value, CompanEdgeProtocol::TimeVal);
    valueSet<struct timeval>(value, arg);

    return VariantValue::set(value);
}
