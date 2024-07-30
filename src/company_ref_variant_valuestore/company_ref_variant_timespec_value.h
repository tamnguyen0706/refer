/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_timespec_value.h
 @brief Derived VariantValue for TimeSpec
 */
#ifndef __company_ref_VARIANT_TIMESPEC_VALUE_H__
#define __company_ref_VARIANT_TIMESPEC_VALUE_H__

#include "company_ref_variant_valuestore_variant.h"

#include <time.h>

namespace Compan{
namespace Edge {

/*!
 * Simplified wrapper class for a VariantValue of
 * CompanEdgeProtocol::TimeSpec
 */
class VariantTimeSpecValue : public VariantValue {
public:
    using Ptr = std::shared_ptr<VariantTimeSpecValue>;

    VariantTimeSpecValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantTimeSpecValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantTimeSpecValue() = default;

    struct timespec get();
    ValueDataSet::Results set(struct timespec const);
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_TIMESPEC_VALUE_H__
