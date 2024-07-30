/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_set_value.h
 @brief Derived VariantValue for Set
 */
#ifndef __company_ref_VARIANT_SET_VALUE_H__
#define __company_ref_VARIANT_SET_VALUE_H__

#include <company_ref_variant_valuestore/company_ref_variant_container_base.h>

#include <set>
#include <string>

namespace Compan{
namespace Edge {

/*!
 * VariantSetValue is a pseudo representation of a std::set
 *
 * CompanValueTypes::SetValue only supports a string representation of a element
 *
 * This class allows for type casting of the internal std::string object to
 * any convertable POD type
 *
 */
class VariantSetValue : public VariantContainerBaseValue<CompanValueTypes::SetValue> {
public:
    using Ptr = std::shared_ptr<VariantSetValue>;

    VariantSetValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);

    VariantSetValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantSetValue() = default;

    template <typename T>
    std::set<T> transformTo() const
    {
        CompanContainerType container(getValue());
        return ValueContainer::transformTo<std::set<T>>(container);
    }

    template <typename T>
    void transformFrom(std::set<T> const& set)
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

#endif // __company_ref_VARIANT_SET_VALUE_H__
