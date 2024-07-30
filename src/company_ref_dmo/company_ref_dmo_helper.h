/**
 Copyright © 2023 COMPAN REF
 @file company_ref_dmo_helper.h
 @brief Helper class for inserting DMO container definitions into a VariantValueStore
 */
#ifndef __company_ref_DMO_HELPER_H__
#define __company_ref_DMO_HELPER_H__

#include "company_ref_dmo_container.h"
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_variant.h>

namespace CompanEdgeProtocol {
class Value;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {

class VariantValueStore;

/*!
 * Helper class for inserting DMO container definitions into a VariantValueStore
 */
class DmoValueStoreHelper {
public:
    DmoValueStoreHelper(DmoContainer const& dmo, VariantValueStore& ws);
    virtual ~DmoValueStoreHelper() = default;

    /*!
     * Creates a container leaf in the VariantValueStore based on a DMO data definition
     *
     * @param parent    Parent location to insert the key
     * @param key       Key value to insert
     * @return  True on success
     */
    bool insertChild(
            ValueId const& parent,
            std::string const& key,
            VariantValue::SetUpdateType const updateType = VariantValue::Local);

    /*!
     * Creates a container leaf in the VariantValueStore based on a DMO data definition
     *
     * @param parent    Parent location to insert the key
     * @param key       Key value to insert
     * @param value     Serialized value
     * @return  True on success
     */
    bool insertChild(
            ValueId const& parent,
            std::string const& key,
            std::string const& value,
            VariantValue::SetUpdateType const updateType = VariantValue::Local);

    /*!
     * Inserts a Value into the VariantValueStore, building up any prior container
     * leafs that are required to create the value
     *
     * @param value
     * @return  True on success
     */
    bool insertValue(
            CompanEdgeProtocol::Value const& value,
            VariantValue::SetUpdateType const updateType = VariantValue::Local);

protected:
    /// Iteratively creates container children based on the DMO data defintion
    void createValuesFromDmo(
            ValueId const& parentId,
            DmoContainer::TreeType const& branch,
            VariantValue::SetUpdateType const updateType);

    /// Locates a DMO definition based on a ValueId path
    DmoContainer::OptionalTreeType locateParentDmo(ValueId const& parentPath);

private:
    DmoContainer const& dmo_;
    VariantValueStore& ws_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_DMO_HELPER_H__
