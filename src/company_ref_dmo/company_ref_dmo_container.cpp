/**
 Copyright © 2023 COMPAN REF
 @file company_ref_dmo_container.cpp
 @brief DataModel Metadata container
 */
#include "company_ref_dmo_container.h"
#include <company_ref_protocol_utils/company_ref_pb_init.h>
#include <company_ref_protocol_utils/company_ref_pb_traits.h>
#include <company_ref_protocol_utils/company_ref_stream.h>

#include <Compan_logger/Compan_logger.h>

namespace Compan{
namespace Edge {
CompanLogger DmoContainerLog("dmo.container", LogLevel::Information);
} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

std::string const DmoContainer::MetaContainerDelim = "+";
ValueId const DmoContainer::MetaContainerDelimId(MetaContainerDelim);

DmoContainer::DmoContainer()
{
}

DmoContainer::~DmoContainer()
{
}

bool DmoContainer::insertValue(CompanEdgeProtocol::Value const& value, bool const metaCheck)
{
    return insertElement(value, metaCheck);
}

void DmoContainer::pruneValues()
{
    std::vector<CompanEdgeProtocol::Value> pruneBucket;

    visitValues([&pruneBucket](CompanEdgeProtocol::Value const& value) { pruneBucket.push_back(value); });

    for (auto& pruneIter : pruneBucket) {
        CompanEdgeProtocol::Value& prune = pruneIter;

        CompanEdgeProtocol::Value_Type type = prune.type();

        if (type == CompanEdgeProtocol::Struct) continue;

        ValueId valueId(prune.id());

        if (type != CompanEdgeProtocol::Container) {
            removeValue(valueId);
            continue;
        }

        boost::optional<TreeType&> parentBranch = dmoTree_.get_child_optional(valueId.name());
        if (parentBranch) {
            std::vector<std::string> pruneChildren;
            for (auto& children : parentBranch.get()) {
                // don't delete a meta definition!
                if (children.first == MetaContainerDelim) continue;

                pruneChildren.push_back(children.first);
            }

            for (auto& pruneChild : pruneChildren) { parentBranch.get().erase(pruneChild); }
        }
    }
}

bool DmoContainer::removeValue(ValueId const& valuePath)
{
    if (valuePath.empty()) return false;

    if (!has(valuePath)) return true;

    ValueId parentPath(valuePath);
    if (!valuePath.parent().empty()) parentPath = valuePath.parent();

    boost::optional<TreeType&> child = dmoTree_.get_child_optional(parentPath.name());
    if (child) {
        size_t count = child.get().erase(valuePath.leaf().name());
        if (child.get().empty()) {

            if (!valuePath.parent().empty())
                return removeValue(valuePath.parent());
            else
                count = dmoTree_.erase(valuePath.name());
        }

        return (count == 1);
    }

    return false;
}

bool DmoContainer::setMetaDataType(ValueId const& valuePath, CompanEdgeProtocol::Value_Type const metaType)
{
    boost::optional<TreeType&> child = dmoTree_.get_child_optional(valuePath.name());
    if (child) {
        valueInit(child.get().data(), metaType);
        return true;
    }

    return false;
}

CompanEdgeProtocol::Value_Type DmoContainer::getMetaDataType(ValueId const& valuePath)
{
    boost::optional<TreeType&> child = dmoTree_.get_child_optional(valuePath.name());
    if (child) { return child.get().data().type(); }

    return CompanEdgeProtocol::Unset;
}

DmoContainer::OptionalTreeType DmoContainer::getMetaData(ValueId const& valuePath) const
{
    OptionalTreeType failureResult;
    if (valuePath.empty()) return failureResult;

    ValueId searchId;
    OptionalTreeType searchChild;
    bool skipAppend(false);
    for (auto iter = valuePath.begin(); iter != valuePath.end(); ++iter) {

        if (!skipAppend) searchId += *iter;

        skipAppend = false;

        searchChild = dmoTree_.get_child_optional(searchId.name());
        if (!searchChild) return failureResult;

        CompanEdgeProtocol::Value const& value = searchChild.get().data();

        if (value.type() == CompanEdgeProtocol::Container) {
            // we know the next value will be a 'key' - replace and skip
            searchId += MetaContainerDelimId;
            skipAppend = true;
        }
    }

    return searchChild;
}

CompanEdgeProtocol::Value DmoContainer::getValueDefinition(ValueId const& valuePath) const
{
    DmoContainer::OptionalTreeType metaData = getMetaData(valuePath);

    if (metaData) return metaData.get().data();

    return CompanEdgeProtocol::Value();
}

void DmoContainer::visitValues(VisitFunction cb) const
{
    ValueId root;
    visitValues(root, dmoTree_, cb);
}

bool DmoContainer::insertMetaContainer(ValueId const& valuePath, CompanEdgeProtocol::Value_Type const containerType)
{
    return insertMetaContainer(valuePath, containerType, CompanEdgeProtocol::Struct);
}

bool DmoContainer::insertMetaContainer(
        ValueId const& valuePath,
        CompanEdgeProtocol::Value_Type const containerType,
        CompanEdgeProtocol::Value_Type const metaType)
{
    // the emplace normalized takes care of the CompanEdgeProtocol::Id value
    CompanEdgeProtocol::Value metaValue;
    valueInit(metaValue, metaType);

    return insertMetaContainer(valuePath, containerType, metaValue);
}

bool DmoContainer::insertMetaContainer(
        ValueId const& valuePath,
        CompanEdgeProtocol::Value_Type const containerType,
        CompanEdgeProtocol::Value const metaValue)
{
    ValueId containerId(valuePath);

    // strip off an extra delim if it's present
    if (containerId.leaf() == MetaContainerDelimId) containerId = containerId.parent();

    CompanEdgeProtocol::Value containerValue;

    valueInit(containerValue, containerType);

    containerValue.set_id(containerId.name());

    if (!insertElement(containerValue, false)) return false;

    ValueId metaDataName(containerId, MetaContainerDelim);

    return emplaceNormalized(metaDataName, metaValue);
}

bool DmoContainer::insertMetaContainer(
        ValueId const& valuePath,
        CompanEdgeProtocol::Value_Type const containerType,
        CompanValueTypes::EnumValue const metaEnumValue)
{
    // the emplace normalized takes care of the CompanEdgeProtocol::Id value
    CompanEdgeProtocol::Value metaValue;
    valueInit(metaValue, CompanEdgeProtocol::Enum);

    *metaValue.mutable_enumvalue() = metaEnumValue;

    return insertMetaContainer(valuePath, containerType, metaValue);
}

bool DmoContainer::insertMetaData(ValueId const& parentPath, CompanEdgeProtocol::Value const& value)
{
    if (value.type() == CompanEdgeProtocol::Container) return insertMetaContainer(parentPath, value.type());

    boost::optional<TreeType&> child = dmoTree_.get_child_optional(parentPath.parent().name());
    if (!child) {
        if (parentPath.parent().leaf() == MetaContainerDelimId)
            insertMetaContainer(parentPath.parent(), CompanEdgeProtocol::Container);
        else {
            /// create parents
            CompanEdgeProtocol::Value structValue;
            valueInit(structValue, CompanEdgeProtocol::Struct);
            structValue.set_id(parentPath.parent().name());
            insertElement(structValue, false);
        }
    }

    CompanEdgeProtocol::Value stripValue(value);
    stripValue.set_id(parentPath.leaf().name());

    if (stripValue.type() != CompanEdgeProtocol::Struct) valueInit(stripValue, stripValue.type());

    return emplaceNormalized(parentPath, stripValue);
}

void DmoContainer::visitMetaData(VisitFunction cb) const
{
    for (auto& pos : dmoTree_) { visitMetaData(ValueId(pos.first), pos.second, cb); }
}

bool DmoContainer::has(ValueId const& valuePath) const
{
    if (valuePath.empty()) return false;

    TreeType branch = dmoTree_;

    for (auto& element : valuePath) {

        TreeType::assoc_iterator iter = branch.find(element.name());

        if (iter == branch.not_found()) return false;

        branch = iter->second;
    }

    return true;
}

bool DmoContainer::isInstance(ValueId const& valuePath) const
{
    if (valuePath.empty()) return false;

    // walk backwards and
    if (valuePath.leaf() == MetaContainerDelimId) return false;

    ValueId parent(valuePath.parent());

    boost::optional<TreeType const&> thisChildren = dmoTree_.get_child_optional(parent.name());
    if (thisChildren) {

        if (!thisChildren.get().empty() && thisChildren.get().begin()->second.data().id() == MetaContainerDelim)
            return true;
    }

    return isInstance(parent);
}

bool DmoContainer::isMetaData(ValueId const& valuePath) const
{
    for (auto& element : valuePath) {
        if (element == MetaContainerDelimId) return true;
    }

    return false;
}

ValueId DmoContainer::getMetaDataPath(ValueId const& valuePath) const
{
    if (valuePath.empty()) return ValueId();

    ValueId searchId;
    OptionalTreeType searchChild;
    bool skipAppend(false);
    for (auto iter = valuePath.begin(); iter != valuePath.end(); ++iter) {

        if (!skipAppend) searchId += *iter;

        skipAppend = false;

        searchChild = dmoTree_.get_child_optional(searchId.name());
        if (!searchChild) return ValueId();

        CompanEdgeProtocol::Value const& value = searchChild.get().data();

        if (value.type() == CompanEdgeProtocol::Container) {
            // we know the next value will be a 'key' - replace and skip
            searchId += MetaContainerDelimId;
            skipAppend = true;
        }
    }

    return searchId;
}

DmoContainer::TreeType const& DmoContainer::root() const
{
    return dmoTree_;
}

bool DmoContainer::empty() const
{
    return dmoTree_.empty();
}

void DmoContainer::print(std::ostream& os)
{
    print(os, dmoTree_, 0);
}

void DmoContainer::print(std::ostream& os, TreeType& pt, int level)
{
    std::string indent(level * 2, ' ');
    for (auto& pos : pt) {

        CompanEdgeProtocol::Value value = pos.second.data();

        std::stringstream strm;

        strm << indent << pos.first << " ( " << CompanEdgeProtocol::Value_Type_Name(value.type()) << " ) ";

        std::string valueSpacer(strm.str().length() < 40 ? 40 - strm.str().length() : strm.str().length(), ' ');

        os << strm.str() << valueSpacer << to_string(value);

        if (value.type() == CompanEdgeProtocol::Container || value.type() == CompanEdgeProtocol::Struct)
            os << " " << value.unknownvalue().value();

        os << std::endl;

        print(os, pos.second, level + 1);
    }
}

bool DmoContainer::emplaceNormalized(ValueId const& valuePath, CompanEdgeProtocol::Value const& value)
{
    CompanEdgeProtocol::Value castedValue(value);

    castedValue.set_id(valuePath.leaf().name());

    if (!ValueTypeTraits::isValid(castedValue)) valueInit(castedValue, value.type());

    dmoTree_.put(valuePath.name(), castedValue);

    return true;
}

bool DmoContainer::insertElement(CompanEdgeProtocol::Value const& value, bool const containerCheck)
{
    ValueId valuePath(value.id());
    ValueId parentPath;

    // walk up the parent path, validating all parent elements are present
    for (auto element = valuePath.begin(); element != valuePath.end(); ++element) {

        parentPath += *element;

        boost::optional<TreeType&> child = dmoTree_.get_child_optional(parentPath.name());
        if (child) {
            CompanEdgeProtocol::Value const& fValue = child.get().data();
            if (containerCheck && (fValue.type() == CompanEdgeProtocol::Container)) return false;

            if (fValue.type() == value.type() && valuePath == parentPath) {
                return emplaceNormalized(parentPath, value);
            }

            continue;
        }

        if (parentPath == valuePath) {

            emplaceNormalized(parentPath, value);

            if (value.type() == CompanEdgeProtocol::Container) {

                ValueId metaDataName(parentPath, MetaContainerDelim);

                // the emplace normalized takes care of the CompanEdgeProtocol::Id value
                CompanEdgeProtocol::Value metaValue;
                valueInit(metaValue, CompanEdgeProtocol::Struct);

                emplaceNormalized(metaDataName, metaValue);
            }

            return true;
        }

        // the emplace normalized takes care of the CompanEdgeProtocol::Id value
        CompanEdgeProtocol::Value parentValue;
        valueInit(parentValue, CompanEdgeProtocol::Struct);

        // If this is a container embedded name (a.b.+.c.container2)
        auto lookAhead = element + 1;
        if (lookAhead != valuePath.end() && *lookAhead == MetaContainerDelimId)
            valueInit(parentValue, CompanEdgeProtocol::Container);

        emplaceNormalized(parentPath, parentValue);
    }

    return false;
}

void DmoContainer::visitValues(ValueId const& parentId, TreeType const& branch, VisitFunction cb) const
{
    for (auto& pos : branch) {

        if (pos.second.data().id() == MetaContainerDelim) continue;

        CompanEdgeProtocol::Value value = pos.second.data();

        ValueId valueId(parentId, pos.second.data().id());
        value.set_id(valueId.name());

        cb(value);

        CompanEdgeProtocol::Value_Type type = value.type();
        if (type == CompanEdgeProtocol::Container || type == CompanEdgeProtocol::Struct)
            visitValues(valueId, pos.second, cb);
    }
}

void DmoContainer::visitMetaData(
        ValueId const& parentId,
        TreeType const& branch,
        VisitFunction cb,
        bool const inMetaData) const
{
    // We only walk until we see a container - which is what would contain definitions
    for (auto& pos : branch) {

        CompanEdgeProtocol::Value value = pos.second.data();

        ValueId valueId(parentId, pos.second.data().id());
        value.set_id(valueId.name());

        if (value.type() == CompanEdgeProtocol::Container) {

            cb(value);

            CompanEdgeProtocol::Value childValue;

            childValue.set_type(CompanEdgeProtocol::Unset);

            boost::optional<TreeType const&> containerChildren = dmoTree_.get_child_optional(valueId.name());
            if (containerChildren) {

                for (auto& children : containerChildren.get()) {

                    // pass over "values" that are defined
                    if (children.first != MetaContainerDelim) continue;

                    childValue = children.second.data();
                    childValue.set_id(ValueId(valueId, children.second.data().id()));
                    break;
                }

                if (childValue.type() != CompanEdgeProtocol::Struct) {

                    cb(childValue);
                    continue;
                }

                visitMetaData(value.id(), pos.second, cb, true);

                continue;
            }

            std::cout << "Whoopsie" << std::endl;
            continue;
        }

        if (value.type() == CompanEdgeProtocol::Struct) {

            // if this is "instance" data - skip it
            if (isInstance(valueId)) continue;

            if (valueId.leaf() != MetaContainerDelim) cb(value);

            visitMetaData(valueId, pos.second, cb, inMetaData);
            continue;
        }

        if (!inMetaData) continue;

        cb(value);
    }
}

void DmoContainer::sort()
{
    sort(dmoTree_);
}

void DmoContainer::sort(TreeType& branch)
{
    branch.sort();

    // We only walk until we see a container - which is what would contain definitions
    for (auto& pos : branch) { sort(pos.second); }
}
