/**
  Copyright © 2023 COMPAN REF
  @file company_ref_variant_valuestore.h
  @brief CompanEdge Variant ValueStore Map
*/

#ifndef __company_ref_VARIANT_VALUESTORE_H__
#define __company_ref_VARIANT_VALUESTORE_H__

#include <company_ref_utils/company_ref_signals.h>

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <mutex>

#include <boost/asio/io_context_strand.hpp>

#include "company_ref_variant_valuestore_value_id_bucketizer.h"
#include "company_ref_variant_valuestore_variant.h"

namespace CompanEdgeProtocol {
class ServerMessage;
class ClientMessage;
class Value;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {

// fwd decl for friendship
class VariantValueDispatcher;

using VariantValueDispatcherPtr = std::shared_ptr<VariantValueDispatcher>;

/*!
 * * @brief VariantValue::Ptr Container
 *
 * Allows for insertion and updates directly from CompanEdgeProtocol::Value objects.
 *
 * Signaling mechanisms for VariantValues being added, changed or remove
 *
 */
class VariantValueStore {
private:
    using WsMapType = std::map<std::string, VariantValue::Ptr>;
    using Iterator = WsMapType::iterator;

public:
    using VisitFunction = std::function<void(VariantValue::Ptr const&)>;

public:
    VariantValueStore(boost::asio::io_context&);
    virtual ~VariantValueStore();

    /*!
     * Adds or updates a CompanEdgeProtocol::Value to the Value Store
     *
     * Requires that the CompanEdgeProtocol::Value has a CompanEdgeProtocol::Value::id and CompanEdgeProtocol::Value::type.
     *
     * - Calls ValueAddSignal on add
     * - Calls ValueChangeSignal on change
     *
     * @note You can not change the "type" of a value in the variant value store once it is added.
     *
     * @param vsValue
     * @param updateType - Identifies who is making the update
     * @return True on success
     */
    bool set(
            CompanEdgeProtocol::Value const& vsValue,
            VariantValue::SetUpdateType const updateType = VariantValue::Local);

    /*!
     * Returns a valid VariantValue::Ptr for a value in the variant value store.
     * @param valueId Value Id
     * @return valid VariantValue::Ptr
     */
    VariantValue::Ptr get(ValueId const& valueId);

    /*!
     * Returns a valid Derived type VariantValue::Ptr for a value in the variant value store.
     * @param valueId Value Id
     * @return valid VariantValue::Ptr
     */
    template <typename Derived>
    typename Derived::Ptr get(ValueId const& valueId)
    {
        return std::dynamic_pointer_cast<Derived>(get(valueId));
    }

    /*!
     * Removes a value from the variant value store
     *
     *  - Calls ValueRemoveSignal on remove
     *
     * @param valueId Value Id
     * @param updateType - Identifies who is making the update
     * @return False if value not found
     */
    bool del(ValueId const& valueId, VariantValue::SetUpdateType const updateType = VariantValue::Local);

    /*!
     * Adds a VariantValue::Ptr into the variant value store
     *
     * - Calls ValueAddSignal on add
     *
     * @param vsValue
     * @return Either the vsValue added, or VariantValue::Ptr() on failure
     */
    VariantValue::Ptr add(VariantValue::Ptr vsValue);

    /*!
     * Returns a valid VariantValue::Ptr for a value in the variant value store.
     * @param valueId Value Id string
     * @return valid VariantValue::Ptr
     */
    VariantValue::Ptr operator[](ValueId const& valueId);

    /// Returns true if the value exists
    bool has(ValueId const& valueId);

    /// Visits all values stored in the value store
    void visitValues(VisitFunction const& visitFunction);

    /// Returns the number of values in the VariantValueStore
    size_t size();

    /// Helper function to return the correct Hash Token used in the Variant Value Store
    HashToken findHashToken(ValueId const&);

    /// Helper function to return the correct ValueId used in the Variant Value Store
    ValueId::Ptr findValueId(HashToken const& hashToken);

    /// Create a AddToContainer value - signals the appropriate handlers
    VariantValue::Ptr addToContainer(
            ValueId const& valueId,
            std::string const& key,
            VariantValue::SetUpdateType const updateType = VariantValue::Local);

    /// Create a AddToContainer value with serialized value - signals the
    /// appropriate handlers
    VariantValue::Ptr addToContainer(
            ValueId const& valueId,
            std::string const& key,
            std::string const& value,
            VariantValue::SetUpdateType const updateType = VariantValue::Local);

    /// Creates a RemoveFromContainer value - signals the
    /// appropriate handlers
    VariantValue::Ptr removeFromContainer(
            ValueId const& valueId,
            std::string const& key,
            VariantValue::SetUpdateType const updateType = VariantValue::Local);

    /// Returns the "root" pointer for parent/child tree
    VariantValue::Ptr root() const;

    void print(std::ostream&);

    boost::asio::io_context::strand& getStrand();

public:
    /// Connects a listener to value added notifications
    SignalConnection connectValueAddedListener(VariantValue::ValueSignal::SlotType const&);
    /// Connects a listener to value changed notifications
    SignalConnection connectValueChangedListener(VariantValue::ValueSignal::SlotType const&);
    /// Connects a listener to value removed notifications
    SignalConnection connectValueRemovedListener(VariantValue::ValueSignal::SlotType const&);
    /// Connects a listener to value AddToContainer notifications
    SignalConnection connectValueAddToContainerListener(VariantValue::ValueSignal::SlotType const&);
    /// Connects a listener to value RemoveFromContainer notifications
    SignalConnection connectValueRemoveFromContainerListener(VariantValue::ValueSignal::SlotType const&);

protected:
    void doAddedSignal(VariantValue::Ptr const);
    void doChangedSignal(VariantValue::Ptr const);
    void doRemovedSignal(VariantValue::Ptr const);
    void doAddToContainerSignal(VariantValue::Ptr const);
    void doRemoveFromContainerSignal(VariantValue::Ptr const);

    /// non-locking get function returns VariantValue::Ptr
    VariantValue::Ptr getSafe(ValueId const& valueId);

    /// Used to create parent Value's when no parent is found
    VariantValue::Ptr getParent(VariantValue::Ptr const valuePtr);

    /// Removes parent and children values
    void delChildren(VariantValue::Ptr, VariantValue::SetUpdateType const updateType);

    bool delContainer(
            VariantValue::Ptr valuePtr,
            VariantValue::SetUpdateType const updateType,
            bool const notifyParentChange = false);

private:
    boost::asio::io_context::strand ctx_;
    ValueIdBucketizer bucketizer_;
    WsMapType wsMap_;

    // root of all children in the ValueStore
    //  Does not get iterated over
    VariantValue::Ptr root_;

    VariantValue::ValueSignal onValueAddedSignal_;
    VariantValue::ValueSignal onValueChangedSignal_;
    VariantValue::ValueSignal onValueRemovedSignal_;
    VariantValue::ValueSignal onValueAddToContainerSignal_;
    VariantValue::ValueSignal onValueRemoveFromContainerSignal_;

    std::mutex mutex_;

    VariantValueDispatcherPtr dataDispatcher_;
};

inline boost::asio::io_context::strand& VariantValueStore::getStrand()
{
    return ctx_;
}

inline SignalConnection VariantValueStore::connectValueAddedListener(VariantValue::ValueSignal::SlotType const& cb)
{
    return onValueAddedSignal_.connect(cb);
}

inline SignalConnection VariantValueStore::connectValueChangedListener(VariantValue::ValueSignal::SlotType const& cb)
{
    return onValueChangedSignal_.connect(cb);
}

inline SignalConnection VariantValueStore::connectValueRemovedListener(VariantValue::ValueSignal::SlotType const& cb)
{
    return onValueRemovedSignal_.connect(cb);
}

inline SignalConnection VariantValueStore::connectValueAddToContainerListener(
        VariantValue::ValueSignal::SlotType const& cb)
{
    return onValueAddToContainerSignal_.connect(cb);
}

inline SignalConnection VariantValueStore::connectValueRemoveFromContainerListener(
        VariantValue::ValueSignal::SlotType const& cb)
{
    return onValueRemoveFromContainerSignal_.connect(cb);
}

inline VariantValue::Ptr VariantValueStore::operator[](ValueId const& valueId)
{
    // ::get() hold's the mutex
    return get(valueId);
}

inline VariantValue::Ptr VariantValueStore::root() const
{
    return root_;
}

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_VARIANT_VALUESTORE_H__*/
