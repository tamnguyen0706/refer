/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_usinterval_value.cpp
 @brief Derived VariantValue for USInterval
 */
#include "company_ref_variant_usinterval_value.h"

#include "company_ref_variant_valuestore_valueid.h"

#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_pb_traits.h>

using namespace Compan::Edge;

VariantUSIntervalValue::VariantUSIntervalValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantIntervalBase(ctx, valueId)
{
}

VariantUSIntervalValue::VariantUSIntervalValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantIntervalBase(ctx, value, updateType)
{
}
