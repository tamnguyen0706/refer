/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_llinterval_value.cpp
 @brief Derived VariantValue for LLInterval
 */
#include "company_ref_variant_llinterval_value.h"

#include "company_ref_variant_valuestore_valueid.h"

#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_pb_traits.h>

using namespace Compan::Edge;

VariantLLIntervalValue::VariantLLIntervalValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantIntervalBase(ctx, valueId)
{
}

VariantLLIntervalValue::VariantLLIntervalValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantIntervalBase(ctx, value, updateType)
{
}
