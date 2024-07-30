/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_ullinterval_value.h
 @brief Derived VariantValue for ULLInterval
 */
#ifndef __company_ref_VARIANT_ULLINTERVAL_VALUE_H__
#define __company_ref_VARIANT_ULLINTERVAL_VALUE_H__

#include "company_ref_variant_interval_base.h"

namespace Compan{
namespace Edge {

/*!
 * Simplified wrapper class for a VariantValue of
 * CompanEdgeProtocol::ULLInterval
 */
class VariantULLIntervalValue : public VariantIntervalBase<uint64_t> {
public:
    using Ptr = std::shared_ptr<VariantULLIntervalValue>;

    VariantULLIntervalValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantULLIntervalValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantULLIntervalValue() = default;
};

} // namespace Edge
} // namespace Compan

#endif //  __company_ref_VARIANT_ULLINTERVAL_VALUE_H__
