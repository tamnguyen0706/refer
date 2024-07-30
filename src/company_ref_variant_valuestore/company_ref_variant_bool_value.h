/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_bool_value.h
 @brief Derived VariantValue for Bool
 */
#ifndef __company_ref_VARIANT_BOOL_VALUE_H__
#define __company_ref_VARIANT_BOOL_VALUE_H__

#include "company_ref_variant_valuestore_variant.h"

namespace Compan{
namespace Edge {

/*!
 * Simplified wrapper class for a VariantValue of
 * CompanEdgeProtocol::Bool
 */
class VariantBoolValue : public VariantValue {
public:
    using Ptr = std::shared_ptr<VariantBoolValue>;

    VariantBoolValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantBoolValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantBoolValue() = default;

    bool get();
    ValueDataSet::Results set(bool const);
};

} // namespace Edge
} // namespace Compan

#endif //__company_ref_VARIANT_BOOL_VALUE_H__
