/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_llinterval_value.h
 @brief Derived VariantValue for LLInterval
 */
#ifndef __company_ref_VARIANT_LLINTERVAL_VALUE_H__
#define __company_ref_VARIANT_LLINTERVAL_VALUE_H__

#include "company_ref_variant_interval_base.h"

namespace Compan{
namespace Edge {

/*!
 * Simplified wrapper class for a VariantValue of
 * CompanEdgeProtocol::LLInterval
 */
class VariantLLIntervalValue : public VariantIntervalBase<int64_t> {
public:
    using Ptr = std::shared_ptr<VariantLLIntervalValue>;

    VariantLLIntervalValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantLLIntervalValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantLLIntervalValue() = default;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_LLINTERVAL_VALUE_H__
