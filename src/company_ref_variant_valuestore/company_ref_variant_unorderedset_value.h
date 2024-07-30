/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_unorderedset_value.h
 @brief Derived VariantValue for Unordered Set
 */
#ifndef __company_ref_VARIANT_UNORDEREDSET_VALUE_H__
#define __company_ref_VARIANT_UNORDEREDSET_VALUE_H__

#include <company_ref_variant_valuestore/company_ref_variant_container_base.h>

#include <string>
#include <unordered_set>

namespace Compan{
namespace Edge {

/*!
 * VariantUnorderedSetValue is a pseudo representation of a std::unordered_set
 *
 * CompanValueTypes::UnorderedSetValue only supports a string representation of a element
 *
 * This class allows for type casting of the internal std::string object to
 * any convertable POD type
 *
 */
class VariantUnorderedSetValue : public VariantContainerBaseValue<CompanValueTypes::UnorderedSetValue> {
public:
    using Ptr = std::shared_ptr<VariantUnorderedSetValue>;

    VariantUnorderedSetValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);

    VariantUnorderedSetValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantUnorderedSetValue() = default;

    template <typename T>
    std::unordered_set<T> transformTo() const
    {
        CompanContainerType container(getValue());
        return ValueContainer::transformTo<std::unordered_set<T>>(container);
    }

    template <typename T>
    void transformFrom(std::unordered_set<T> const& set)
    {
        setValue(ValueContainer::transformFrom<CompanContainerType>(set));
    }

protected:
    virtual bool setValue(CompanContainerType const& container);
    virtual CompanContainerType getValue();
    virtual CompanContainerType getValue() const;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_UNORDEREDSET_VALUE_H__
