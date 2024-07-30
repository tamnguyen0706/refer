/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_vector_value.h
 @brief Derived VariantValue for Vector
 */
#ifndef __company_ref_VARIANT_VECTOR_VALUE_H__
#define __company_ref_VARIANT_VECTOR_VALUE_H__

#include <company_ref_variant_valuestore/company_ref_variant_container_base.h>

#include <string>
#include <vector>

namespace Compan{
namespace Edge {

/*!
 * VariantVectorValue is a pseudo representation of a std::vector
 *
 * CompanValueTypes::VectorValue only supports a string representation of a element
 *
 * This class allows for type casting of the internal std::string object to
 * any convertable POD type
 *
 */
class VariantVectorValue : public VariantContainerBaseValue<CompanValueTypes::VectorValue> {
public:
    using Ptr = std::shared_ptr<VariantVectorValue>;

    VariantVectorValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);

    VariantVectorValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantVectorValue() = default;

    // template <typename T>
    // T operator[](int const index) const
    // how would this work with a pointer? *p<int>[0] ?

    template <typename T>
    T at(int const index) const
    {
        CompanContainerType container(getValue());
        return ValueContainer::at<T>(container, index);
    }

    template <typename T>
    std::vector<T> transformTo() const
    {
        CompanContainerType container(getValue());
        return ValueContainer::transformTo<std::vector<T>>(container);
    }

    template <typename T>
    void transformFrom(std::vector<T> const& vector)
    {
        setValue(ValueContainer::transformFrom<CompanContainerType>(vector));
    }

protected:
    virtual bool setValue(CompanContainerType const& container);
    virtual CompanContainerType getValue();
    virtual CompanContainerType getValue() const;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_VECTOR_VALUE_H__
