/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_bool_value.cpp
 @brief Derived VariantValue for Bool
 */

#include "company_ref_variant_bool_value.h"

#include "company_ref_variant_valuestore_valueid.h"

#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_pb_traits.h>

using namespace Compan::Edge;

VariantBoolValue::VariantBoolValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantValue(ctx, valueId, CompanEdgeProtocol::Unset)
{
}

VariantBoolValue::VariantBoolValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantValue(ctx, value, updateType)
{
}

bool VariantBoolValue::get()
{
    return valueGet<bool>(VariantValue::get());
}

ValueDataSet::Results VariantBoolValue::set(bool const arg)
{
    CompanEdgeProtocol::Value value;
    valueInit(value, CompanEdgeProtocol::Bool);
    valueSet<bool>(value, arg);

    return VariantValue::set(value);
}
