/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_visitor.cpp
 @brief VariantValue Child visitor function
 */
#include "company_ref_variant_valuestore_visitor.h"

#include "company_ref_variant_valuestore_valueid.h"

using namespace Compan::Edge;

void VariantValueVisitor::visitChildren(VariantValue::Ptr valuePtr, VisitFunction const& visitFunction)
{
    if (valuePtr == nullptr) return;

    for (auto& children : valuePtr->getChildren()) {
        VariantValue::Ptr childPtr = children.second;

        if (childPtr == nullptr) continue;

        visitFunction(childPtr);
        if (childPtr->hasChildren()) visitChildren(children.second, visitFunction);
    }
}

void VariantValueVisitor::visitParent(VariantValue::Ptr valuePtr, VisitFunction const& visitFunction)
{
    if (valuePtr == nullptr) return;

    VariantValue::Ptr parent = valuePtr->parent();
    if (parent == nullptr || parent->id().empty()) return;

    visitFunction(parent);
    visitParent(parent, visitFunction);
}
