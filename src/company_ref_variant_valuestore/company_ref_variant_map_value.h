/**
 Copyright © 2024 COMPAN REF
 @file company_ref_variant_map_value.h
 @brief Derived VariantValue for MapValue
 */
#ifndef __company_ref_VARIANT_MAP_VALUE_H__
#define __company_ref_VARIANT_MAP_VALUE_H__

#include "company_ref_variant_valuestore_variant.h"

namespace Compan{
namespace Edge {

class VariantValueStore;

/*!
 * VariantMapValue is a pseudo representation of
 * a std::map.
 *
 * Insertion requires a AddToContainer message to
 * a VariantValueStore server, which in turn will
 * respond back with the elements added.
 *
 * Removal requires a RemoveFromContainer message to a
 * VariantValueStore server, which in turn will
 * respond back with the elements removed, which is
 * redundant, as this container will correctly remove
 * an indexed element
 *
 * Add/Remove notifications should be captured
 * by connectChangedListener.
 *
 * The underlaying mechanism used to represent a
 * map container exists via the VariantValue's
 * parent/child relationship of values.
 *
 * The immediate child is the key index for
 * elements represented as a map
 *
 */
class VariantMapValue : public VariantValue {
public:
    using Ptr = std::shared_ptr<VariantMapValue>;
    using Iterator = ChildMapType::const_iterator;

    VariantMapValue(boost::asio::io_context::strand& ctx, ValueId const& valueId);
    VariantMapValue(
            boost::asio::io_context::strand& ctx,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantMapValue() = default;

    /// Clears the children
    void clear();

    /// Checks whether the map is empty
    bool empty() const;

    /// Returns the number of elements
    size_t size() const;

    /// Returns an iterator to the beginning
    Iterator begin();

    /// Returns an iterator to the end
    Iterator end();

    // These issue a AddToContainer message
    bool insert(int const&);
    bool insert(ValueId const&);

    // These issue a RemoveFromContainer message
    bool erase(int const&);
    bool erase(ValueId const&);
    Iterator erase(Iterator);

    /// Finds element with specific key, and returns a valid iterator
    Iterator find(int const&);
    Iterator find(ValueId const&);

    /// Returns an VariantValue::Ptr for a speicific key, or a nullptr
    VariantValue::Ptr at(int const&);
    VariantValue::Ptr at(ValueId const&);

    SignalConnection connectValueAddToContainerListener(VariantValue::ValueSignal::SlotType const& cb);

    SignalConnection connectValueRemoveFromContainerListener(VariantValue::ValueSignal::SlotType const& cb);

    /// sends out the add container signal on demand
    void addToContainer(ValueId const& key);

    /// sends out the add container signal on demand
    void removeFromContainer(ValueId const& key);

protected:
    // signal intercept - forward it one to the correct handlers
    virtual void signal(VariantValue::Ptr);

    friend class VariantValueStore;

    /// Connects the WS AddToContainer notification signal function for direct callback
    void setWsAddToContainerSignal(ValueSignal::SlotType const&);

    /// Connects the WS RemoveFromContainer notification signal function for direct callback
    void setWsRemoveFromContainerSignal(ValueSignal::SlotType const&);

private:
    VariantValue::ValueSignal onAddToContainer_;
    VariantValue::ValueSignal onRemoveFromContainer_;

    // used to bypass signal -> signal delays
    ValueSignal::SlotType wsAddToContainerNotification_;
    ValueSignal::SlotType wsRemoveFromContainerNotification_;
};

inline SignalConnection VariantMapValue::connectValueAddToContainerListener(
        VariantValue::ValueSignal::SlotType const& cb)
{
    return onAddToContainer_.connect(cb);
}

inline SignalConnection VariantMapValue::connectValueRemoveFromContainerListener(
        VariantValue::ValueSignal::SlotType const& cb)
{
    return onRemoveFromContainer_.connect(cb);
}

inline void VariantMapValue::setWsAddToContainerSignal(ValueSignal::SlotType const& cb)
{
    wsAddToContainerNotification_ = cb;
}

inline void VariantMapValue::setWsRemoveFromContainerSignal(ValueSignal::SlotType const& cb)
{
    wsRemoveFromContainerNotification_ = cb;
}

} // namespace Edge
} // namespace Compan

#endif //__company_ref_VARIANT_MAP_VALUE_H__
