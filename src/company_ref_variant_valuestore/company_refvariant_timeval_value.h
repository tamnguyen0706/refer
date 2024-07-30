/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_timeval_value.h
 @brief Derived VariantValue for TimeVal
 */
#ifndef __company_ref_VARIANT_TIMEVAL_VALUE_H__
#define __company_ref_VARIANT_TIMEVAL_VALUE_H__

#include "company_ref_variant_valuestore_variant.h"

#include <time.h>

namespace Compan{
namespace Edge {

/*!
 * Simplified wrapper class for a VariantValue of
 * CompanEdgeProtocol::TimeVal
 */
class VariantTimeValValue : public VariantValue {
public:
    using Ptr = std::shared_ptr<VariantTimeValValue>;

    VariantTimeValValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantTimeValValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantTimeValValue() = default;

    struct timeval get();
    ValueDataSet::Results set(struct timeval const);
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_TIMEVAL_VALUE_H__
