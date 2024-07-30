/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_sinterval_value.h
 @brief Derived VariantValue for SInterval
 */
#ifndef __company_ref_VARIANT_SINTERVAL_VALUE_H__
#define __company_ref_VARIANT_SINTERVAL_VALUE_H__

#include "company_ref_variant_interval_base.h"

namespace Compan{
namespace Edge {

/*!
 * Simplified wrapper class for a VariantValue of
 * CompanEdgeProtocol::SInterval
 */
class VariantSIntervalValue : public VariantIntervalBase<int16_t> {
public:
    using Ptr = std::shared_ptr<VariantSIntervalValue>;

    VariantSIntervalValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantSIntervalValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantSIntervalValue() = default;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_SINTERVAL_VALUE_H__
