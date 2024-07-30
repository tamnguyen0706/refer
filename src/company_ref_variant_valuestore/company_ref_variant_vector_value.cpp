/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_vector_value.cpp
 @brief Derived VariantValue for Vector
 */

#include "company_ref_variant_vector_value.h"

#include <company_ref_protocol_utils/company_ref_pb_init.h>

using namespace Compan::Edge;

VariantVectorValue::VariantVectorValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantContainerBaseValue<CompanValueTypes::VectorValue>(ctx, valueId)
{
}

VariantVectorValue::VariantVectorValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantContainerBaseValue<CompanValueTypes::VectorValue>(ctx, value, updateType)
{
}

bool VariantVectorValue::setValue(CompanContainerType const& container)
{
    CompanEdgeProtocol::Value value;
    valueInit(value, CompanEdgeProtocol::Vector);

    *value.mutable_vectorvalue() = std::move(container);

    ValueDataSet::Results result = VariantValue::set(value);

    return (result == ValueDataSet::Success || result == ValueDataSet::SameValue);
}

VariantVectorValue::CompanContainerType VariantVectorValue::getValue()
{
    return VariantValue::get().vectorvalue();
}

VariantVectorValue::CompanContainerType VariantVectorValue::getValue() const
{
    return VariantValue::get().vectorvalue();
}
