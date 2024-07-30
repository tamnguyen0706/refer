/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_unorderedset_value.cpp
 @brief Derived VariantValue for Unordered Set
 */
#include "company_ref_variant_unorderedset_value.h"
#include <company_ref_protocol_utils/company_ref_pb_init.h>

using namespace Compan::Edge;

VariantUnorderedSetValue::VariantUnorderedSetValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantContainerBaseValue<CompanValueTypes::UnorderedSetValue>(ctx, valueId)
{
}

VariantUnorderedSetValue::VariantUnorderedSetValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantContainerBaseValue<CompanValueTypes::UnorderedSetValue>(ctx, value, updateType)
{
}

bool VariantUnorderedSetValue::setValue(CompanContainerType const& container)
{
    CompanEdgeProtocol::Value value;
    valueInit(value, CompanEdgeProtocol::UnorderedSet);

    *value.mutable_unorderedsetvalue() = std::move(container);

    ValueDataSet::Results result = VariantValue::set(value);

    return (result == ValueDataSet::Success || result == ValueDataSet::SameValue);
}

VariantUnorderedSetValue::CompanContainerType VariantUnorderedSetValue::getValue()
{
    return VariantValue::get().unorderedsetvalue();
}

VariantUnorderedSetValue::CompanContainerType VariantUnorderedSetValue::getValue() const
{
    return VariantValue::get().unorderedsetvalue();
}
