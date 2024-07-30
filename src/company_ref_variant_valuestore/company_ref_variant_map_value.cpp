/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_map_value.cpp
 @brief Derived VariantValue for MapValue
 */
#include "company_ref_variant_map_value.h"

#include "company_ref_variant_container_util.h"
#include "company_ref_variant_valuestore_valueid.h"

using namespace Compan::Edge;

VariantMapValue::VariantMapValue(boost::asio::io_context::strand& ctx, ValueId const& valueId)
    : VariantValue(ctx, valueId, CompanEdgeProtocol::Unset)
    , onAddToContainer_(ctx)
    , onRemoveFromContainer_(ctx)
{
}

VariantMapValue::VariantMapValue(
        boost::asio::io_context::strand& ctx,
        CompanEdgeProtocol::Value const& value,
        SetUpdateType const updateType)
    : VariantValue(ctx, value, updateType)
    , onAddToContainer_(ctx)
    , onRemoveFromContainer_(ctx)
{
}

void VariantMapValue::clear()
{
    for (auto iter = begin(); iter != end(); ++iter) erase(iter);
}

bool VariantMapValue::empty() const
{
    return !hasChildren();
}

size_t VariantMapValue::size() const
{
    return children_.size();
}

VariantMapValue::Iterator VariantMapValue::begin()
{
    return children_.begin();
}

VariantMapValue::Iterator VariantMapValue::end()
{
    return children_.end();
}

bool VariantMapValue::insert(int const& key)
{
    return insert(std::to_string(key));
}

bool VariantMapValue::insert(ValueId const& key)
{
    if (getChild(key) != nullptr) return false;

    VariantValue::Ptr addValuePtr = VariantContainerUtil::makeAddToContainer(shared_from_this(), key, Local);
    if (addValuePtr == nullptr) return false;

    signal(addValuePtr);
    signal(shared_from_this());

    return true;
}

bool VariantMapValue::erase(int const& key)
{
    return erase(std::to_string(key));
}

bool VariantMapValue::erase(ValueId const& key)
{
    if (getChild(key) == nullptr) return false;

    VariantValue::Ptr removeValuePtr = VariantContainerUtil::makeRemoveFromContainer(shared_from_this(), key, Local);
    if (removeValuePtr == nullptr) return false;

    signal(removeValuePtr);
    signal(shared_from_this());

    return true;
}

VariantMapValue::Iterator VariantMapValue::erase(Iterator pos)
{
    if (erase(pos->first)) return ++pos;

    return children_.end();
}

VariantMapValue::Iterator VariantMapValue::find(int const& key)
{
    return find(std::to_string(key));
}

VariantMapValue::Iterator VariantMapValue::find(ValueId const& key)
{
    return children_.find(key);
}

VariantValue::Ptr VariantMapValue::at(int const& key)
{
    return at(std::to_string(key));
}

VariantValue::Ptr VariantMapValue::at(ValueId const& key)
{
    auto iter = children_.find(key);
    if (iter == children_.end()) return nullptr;

    return iter->second;
}

void VariantMapValue::signal(VariantValue::Ptr valuePtr)
{
    if (valuePtr->type() == CompanEdgeProtocol::ContainerAddTo) {
        if (wsAddToContainerNotification_) wsAddToContainerNotification_(valuePtr);

        onAddToContainer_(valuePtr);
    } else if (valuePtr->type() == CompanEdgeProtocol::ContainerRemoveFrom) {
        if (wsRemoveFromContainerNotification_) wsRemoveFromContainerNotification_(valuePtr);

        onRemoveFromContainer_(valuePtr);
    } else
        VariantValue::signal(valuePtr);
}

void VariantMapValue::addToContainer(ValueId const& key)
{
    VariantValue::Ptr addValuePtr = VariantContainerUtil::makeAddToContainer(shared_from_this(), key, Local);
    if (addValuePtr == nullptr) return;
    signal(addValuePtr);
}

void VariantMapValue::removeFromContainer(ValueId const& key)
{
    VariantValue::Ptr removeValuePtr = VariantContainerUtil::makeRemoveFromContainer(shared_from_this(), key, Local);
    if (removeValuePtr == nullptr) return;
    signal(removeValuePtr);
}
