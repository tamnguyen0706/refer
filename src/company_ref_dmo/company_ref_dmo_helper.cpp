/**
 Copyright © 2023 COMPAN REF
 @file company_ref_dmo_helper.cpp
 @brief Helper class for inserting DMO container definitions into a VariantValueStore
 */

#include "company_ref_dmo_helper.h"

#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_utils/company_ref_regex_utils.h>
#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>
#include <Compan_logger/Compan_logger.h>

namespace Compan{
namespace Edge {
CompanLogger DmoValueStoreHelperLog("dmo.metacreator", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

DmoValueStoreHelper::DmoValueStoreHelper(DmoContainer const& dmo, VariantValueStore& ws)
    : dmo_(dmo)
    , ws_(ws)
{
}

bool DmoValueStoreHelper::insertChild(
        ValueId const& parent,
        std::string const& key,
        VariantValue::SetUpdateType const updateType)
{
    return insertChild(parent, key, "", updateType);
}

bool DmoValueStoreHelper::insertChild(
        ValueId const& parent,
        std::string const& key,
        std::string const& value,
        VariantValue::SetUpdateType const updateType)
{
    VariantMapValue::Ptr valuePtr = ws_.get<VariantMapValue>(parent);
    if (valuePtr == nullptr) {
        ErrorLog(DmoValueStoreHelperLog) << "Not a container: " << parent << std::endl;
        return false;
    }

    ValueId containerId(parent, key);

    DmoContainer::OptionalTreeType metaBranch = dmo_.getMetaData(parent);
    if (metaBranch) {

        if (metaBranch.get().data().type() != CompanEdgeProtocol::Container) {
            ErrorLog(DmoValueStoreHelperLog) << "Invalid meta data container: " << containerId << std::endl;
            return false;
        }

        DmoContainer::TreeType::const_iterator metaChild = metaBranch.get().begin();

        CompanEdgeProtocol::Value value = (*metaChild).second.data();

        value.set_id(containerId.name());
        if (!ws_.set(value, updateType)) {
            ErrorLog(DmoValueStoreHelperLog) << "Invalid meta data value: " << value << std::endl;
            return false;
        }

        if (value.type() == CompanEdgeProtocol::Struct) createValuesFromDmo(containerId, (*metaChild).second, updateType);
    }

    valuePtr->addToContainer(key);

    if (!value.empty()) {
        // format is:
        //  { childKey=value,childKey=value }
        //

        size_t firstBracePos = value.find_first_of('{');
        size_t lastBracePos = value.find_last_of('}');

        if (firstBracePos == std::string::npos || lastBracePos == std::string::npos) {
            ErrorLog(DmoValueStoreHelperLog) << "Malformatted curly brace: " << value << std::endl;
            return true;
        }

        size_t begin = firstBracePos + 1;
        size_t end = lastBracePos + 1;
        while (begin < end) {
            size_t comma = value.find(',', begin);
            if (comma == std::string::npos) comma = lastBracePos;

            std::string argPair(value.substr(begin, comma - begin));
            begin = comma + 1;

            size_t assignPos = argPair.find('=');

            if (assignPos == std::string::npos) {
                ErrorLog(DmoValueStoreHelperLog) << "Malformatted value: " << argPair << std::endl;
                continue;
            }

            ValueId childId(containerId, std::regex_replace(argPair.substr(0, assignPos), RegexUtils::TrimPattern, ""));

            if (!ws_.has(childId)) {
                ErrorLog(DmoValueStoreHelperLog) << "Value not found: " << childId << std::endl;
                continue;
            }

            ws_.get(childId)->set(
                    std::regex_replace(argPair.substr(assignPos + 1), RegexUtils::TrimPattern, ""), updateType);
        }
    }
    return true;
}

void DmoValueStoreHelper::createValuesFromDmo(
        ValueId const& parentId,
        DmoContainer::TreeType const& branch,
        VariantValue::SetUpdateType const updateType)
{
    // iteraterate over the elements found in a branch and create them in the VariantValueStore
    for (auto& pos : branch) {

        CompanEdgeProtocol::Value value(pos.second.data());
        ValueId valueId(parentId, value.id());

        if (!ws_.has(valueId)) {
            value.set_id(valueId.name());
            if (!ws_.set(value, updateType)) {
                ErrorLog(DmoValueStoreHelperLog) << "Invalid meta data value: " << value << std::endl;
                return;
            }
        }

        // Walk into a struct value and do the same thing
        if (value.type() == CompanEdgeProtocol::Struct) { createValuesFromDmo(valueId, pos.second, updateType); }
    }
}

bool DmoValueStoreHelper::insertValue(
        CompanEdgeProtocol::Value const& value,
        VariantValue::SetUpdateType const updateType)
{
    ValueId valueId(value.id());

    VariantValue::Ptr valuePtr = ws_.get(valueId);
    if (valuePtr) {
        valuePtr->set(value);
        return true;
    }

    // Walks the value id name forward, building up parent elements as it goes
    ValueId parentWalkId;
    for (auto& element : valueId) {
        parentWalkId += element;
        if (ws_.has(parentWalkId)) continue;

        DmoContainer::OptionalTreeType metaBranch = locateParentDmo(parentWalkId);
        if (metaBranch) {

            CompanEdgeProtocol::Value metaValue = metaBranch.get().data();

            if (element == metaValue.id()) {
                if (metaValue.type() == CompanEdgeProtocol::Unset) metaValue.set_type(CompanEdgeProtocol::Struct);

                metaValue.set_id(parentWalkId);
                if (!ws_.set(metaValue, updateType))
                    ErrorLog(DmoValueStoreHelperLog) << "Invalid meta data value: " << metaValue << std::endl;

                continue;
            }

            DmoContainer::TreeType::const_iterator metaChild = metaBranch.get().begin();

            metaValue = (*metaChild).second.data();

            if (metaValue.type() == CompanEdgeProtocol::Struct) {
                // create the base parent
                metaValue.set_id(parentWalkId);
                if (!ws_.set(metaValue, updateType))
                    ErrorLog(DmoValueStoreHelperLog) << "Invalid meta data value: " << metaValue << std::endl;

                createValuesFromDmo(parentWalkId, (*metaChild).second, updateType);
            } else {
                metaValue.set_id(parentWalkId);
                if (!ws_.set(metaValue, updateType))
                    ErrorLog(DmoValueStoreHelperLog) << "Invalid meta data value: " << metaValue << std::endl;
            }
        }
    }

    valuePtr = ws_.get(valueId);
    if (valuePtr) {
        valuePtr->set(value, updateType);
        return true;
    }

    return false;
}

DmoContainer::OptionalTreeType DmoValueStoreHelper::locateParentDmo(ValueId const& parentPath)
{
    // Walks the element backward until it finds a name match
    ValueId parentId(parentPath);

    while (!parentId.empty()) {
        DmoContainer::OptionalTreeType metaBranch = dmo_.getMetaData(parentId);

        if (metaBranch && parentId.leaf() == metaBranch.get().data().id()) return metaBranch;

        if (parentId.parent() != parentId)
            parentId = parentId.parent();
        else
            break;
    }

    return DmoContainer::OptionalTreeType();
}
