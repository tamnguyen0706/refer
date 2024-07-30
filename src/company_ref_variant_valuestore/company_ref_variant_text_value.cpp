/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_text_value.cpp
 @brief Derived VariantValue for Text
 */

#include "company_ref_variant_text_value.h"

#include "company_ref_variant_valuestore_valueid.h"

#include <company_ref_protocol_utils/company_ref_pb_accesors.h>
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_pb_traits.h>

using namespace Compan::Edge;

VariantTextValue::VariantTextValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantValue(ctx, valueId, CompanEdgeProtocol::Unset)
{
}

VariantTextValue::VariantTextValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantValue(ctx, value, updateType)
{
}

std::string VariantTextValue::get()
{
    return VariantValue::str();
}

ValueDataSet::Results VariantTextValue::set(std::string const& arg)
{
    return VariantValue::set(arg);
}
