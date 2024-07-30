/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_visitor.h
 @brief VariantValue Child visitor function
 */
#ifndef __company_ref_VARIANT_VALUESTORE_VISITOR_H__
#define __company_ref_VARIANT_VALUESTORE_VISITOR_H__

#include "company_ref_variant_valuestore_variant.h"

namespace Compan{
namespace Edge {

/*!
 * @brief
 */
class VariantValueVisitor {
public:
    using VisitFunction = std::function<void(VariantValue::Ptr const&)>;
    static void visitChildren(VariantValue::Ptr valuePtr, VisitFunction const& visitFunction);
    static void visitParent(VariantValue::Ptr valuePtr, VisitFunction const& visitFunction);
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_VALUESTORE_VISITOR_H__
