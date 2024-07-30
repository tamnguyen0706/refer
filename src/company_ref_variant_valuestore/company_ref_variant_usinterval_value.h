/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_usinterval_value.h
 @brief Derived VariantValue for USInterval
 */
#ifndef __company_ref_VARIANT_USINTERVAL_VALUE_H__
#define __company_ref_VARIANT_USINTERVAL_VALUE_H__

#include "company_ref_variant_interval_base.h"

namespace Compan{
namespace Edge {

/*!
 * Simplified wrapper class for a VariantValue of
 * CompanEdgeProtocol::USInterval
 */
class VariantUSIntervalValue : public VariantIntervalBase<uint16_t> {
public:
    using Ptr = std::shared_ptr<VariantUSIntervalValue>;

    VariantUSIntervalValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantUSIntervalValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantUSIntervalValue() = default;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_USINTERVAL_VALUE_H__
