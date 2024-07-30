/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_uinterval_value.h
 @brief Derived VariantValue for UInterval
 */
#ifndef __company_ref_VARIANT_UINTERVAL_VALUE_H__
#define __company_ref_VARIANT_UINTERVAL_VALUE_H__

#include "company_ref_variant_interval_base.h"

namespace Compan{
namespace Edge {

/*!
 * Simplified wrapper class for a VariantValue of
 * CompanEdgeProtocol::UInterval
 */
class VariantUIntervalValue : public VariantIntervalBase<uint32_t> {
public:
    using Ptr = std::shared_ptr<VariantUIntervalValue>;

    VariantUIntervalValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantUIntervalValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantUIntervalValue() = default;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_UINTERVAL_VALUE_H__
