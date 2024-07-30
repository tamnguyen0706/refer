/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_udid_value.h
 @brief Derived VariantValue for Udid
 */
#ifndef __company_ref_VARIANT_UDID_VALUE_H__
#define __company_ref_VARIANT_UDID_VALUE_H__

#include "company_ref_variant_valuestore_variant.h"

namespace Compan{
namespace Edge {

/*!
 * Simplified wrapper class for a VariantValue of
 * CompanEdgeProtocol::Udid
 */
class VariantUdidValue : public VariantValue {
public:
    using Ptr = std::shared_ptr<VariantUdidValue>;

    VariantUdidValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantUdidValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantUdidValue() = default;

    uint64_t get();
    ValueDataSet::Results set(uint64_t const);
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_UDID_VALUE_H__
