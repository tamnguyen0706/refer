/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_text_value.h
 @brief Derived VariantValue for Text
 */
#ifndef __company_ref_VARIANT_TEXT_VALUE_H__
#define __company_ref_VARIANT_TEXT_VALUE_H__

#include "company_ref_variant_valuestore_variant.h"

namespace Compan{
namespace Edge {

/*!
 * Simplified wrapper class for a VariantValue of
 * CompanEdgeProtocol::Text
 */
class VariantTextValue : public VariantValue {
public:
    using Ptr = std::shared_ptr<VariantTextValue>;

    VariantTextValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantTextValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantTextValue() = default;

    std::string get();
    ValueDataSet::Results set(std::string const&);
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_TEXT_VALUE_H__
