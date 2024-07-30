/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_set_value.cpp
 @brief Derived VariantValue for Set
 */

#include "company_ref_variant_set_value.h"

#include <company_ref_protocol_utils/company_ref_pb_init.h>

using namespace Compan::Edge;

VariantSetValue::VariantSetValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantContainerBaseValue<CompanValueTypes::SetValue>(ctx, valueId)
{
}

VariantSetValue::VariantSetValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantContainerBaseValue<CompanValueTypes::SetValue>(ctx, value, updateType)
{
}

bool VariantSetValue::setValue(CompanContainerType const& container)
{
    CompanEdgeProtocol::Value value;
    valueInit(value, CompanEdgeProtocol::Set);

    *value.mutable_setvalue() = std::move(container);

    ValueDataSet::Results result = VariantValue::set(value);

    return (result == ValueDataSet::Success || result == ValueDataSet::SameValue);
}

VariantSetValue::CompanContainerType VariantSetValue::getValue()
{
    return VariantValue::get().setvalue();
}

VariantSetValue::CompanContainerType VariantSetValue::getValue() const
{
    return VariantValue::get().setvalue();
}
