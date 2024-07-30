/**
  Copyright © 2023 COMPAN REF
  @file company_ref_variant_valuestore_variant.h
  @brief CompanEdge Variant Value
*/

#ifndef __company_ref_VARIANT_VALUESTORE_VARIANT_H__
#define __company_ref_VARIANT_VALUESTORE_VARIANT_H__

#include <company_ref_utils/company_ref_signals.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore_valuedata.h>

#include <map>
#include <memory>
#include <set>

using namespace CompanValueTypes;

// CompanEdge Protocol fwd declarations
namespace CompanEdgeProtocol {
enum Value_Type : int;
class Value;
} // namespace CompanEdgeProtocol

namespace Compan{
namespace Edge {

// fwd decl for friendship
class ValueId;
class VariantValueDispatcher;
class VariantValueStore;

using ValueIdPtr = std::shared_ptr<ValueId>;
using VariantValueDispatcherPtr = std::shared_ptr<VariantValueDispatcher>;

/*!
 * @brief Encapsulation of CompanEdgeProtocol::Value
 *
 * - Allows setting the data from another CompanEdgeProtocol::Value or a std::string
 * - Signals consumers when data has changed
 */
class VariantValue : public std::enable_shared_from_this<VariantValue> {
public:
    using Ptr = std::shared_ptr<VariantValue>;

    // signal for Add/Change/Remove type notifications
    using ValueSignal = SignalAsioStrand<void(Ptr const)>;

    /// The ChildMapType only holds the leaf name ValueId
    using ChildMapType = std::map<ValueId, VariantValue::Ptr>;

    /*!
     * SetUpdateType allows the Value to control how the
     * value is being set, and allows for the ChangeSignal
     * handlers to decide what to do with the value.
     *
     * This allows for control in a mirror, so that values
     * are not constantly ping-ponged on updates
     */
    enum SetUpdateType {
        Local, //!< Default sets
        Remote //!< Coming from a mirrored VariantValueStore
    };

    /*!
     * Constructor Read/Write value
     *
     * @param valueId   Value Id string
     * @param valueType Fundamental data type
     */
    VariantValue(
            boost::asio::io_context::strand&,
            ValueId const& valueId,
            CompanEdgeProtocol::Value_Type const valueType);

    /*!
     *
     * @param valueId       Value Id string
     * @param valueType     Fundamental data type
     * @param valueAccess   Read access type
     */
    VariantValue(
            boost::asio::io_context::strand&,
            ValueId const& valueId,
            CompanEdgeProtocol::Value_Type const valueType,
            CompanEdgeProtocol::Value_Access const valueAccess);

    /*!
     * Constructs a EnumValue Variant with default enumerators
     *
     * @param valueId       Value Id string
     * @param valueAccess   Read access type
     * @param enumerator    Enumerator values
     *
     */
    VariantValue(
            boost::asio::io_context::strand&,
            ValueId const& valueId,
            CompanEdgeProtocol::Value_Access const valueAccess,
            std::set<std::pair<uint32_t, std::string>> const& enumerator);

    /*!
     * Creates a VariantValue from a CompanEdgeProtocol::Value
     *
     * Requires that the CompanEdgeProtocol::Value has a CompanEdgeProtocol::Value::id and CompanEdgeProtocol::Value::type.
     *
     * @param value Fully defined CompanEdgeProtocol::Value
     */
    VariantValue(
            boost::asio::io_context::strand&,
            CompanEdgeProtocol::Value const& value,
            SetUpdateType const updateType = Local);

    virtual ~VariantValue();

    /// Set's the global data dispatcher
    void setDataDispatcher(VariantValueDispatcherPtr dataDispatcher);

    /// Returns the CompanEdgeProtocol::Value::id
    ValueId const id() const;

    /// Returns the CompanEdgeProtocol::Value::type
    CompanEdgeProtocol::Value_Type type() const;

    /// Returns the CompanEdgeProtocol::Value::hashtoken
    uint64_t hashToken() const;

    /// Sets the CompanEdgeProtocol::Value::hashtoken
    void hashToken(uint64_t const&);

    /// Returns the CompanEdgeProtocol::Value::access
    CompanEdgeProtocol::Value_Access access() const;

    /// Sets the CompanEdgeProtocol::Value::access
    void access(CompanEdgeProtocol::Value_Access const);

    /// Returns the last set update type
    SetUpdateType setUpdateType() const;

    /// Returns a copy of the CompanEdgeProtocol::Value object
    CompanEdgeProtocol::Value get() const;

    /*!
     * Attempts to transform the string data to the correct under laying data type.
     *
     * - Calls ChangeSignal on Success
     *
     * @param arg           string data to set
     * @param updateType    Sets who the updater was
     * @return SetResults value
     */
    ValueDataSet::Results set(std::string const& arg, SetUpdateType const updateType = Local);

    /*!
     * Attempts to set the under laying data from another CompanEdgeProtocol::Value object.
     *
     * - Calls ChangeSignal on Success
     *
     * @param arg           string data to set
     * @param updateType    Sets who the updater was
     * @return SetResults value
     */
    ValueDataSet::Results set(CompanEdgeProtocol::Value const& arg, SetUpdateType const updateType = Local);

    /// Returns true data has been set
    bool hasData();

    /// Returns a string representation of the data
    std::string str() const;

    /// Connects a listener to value changed notifications
    SignalConnection connectChangedListener(ValueSignal::SlotType const&);

    /// Returns the parent VariantValue::Ptr
    VariantValue::Ptr parent();

    /// Returns true if the VariantValue has children
    bool hasChildren() const;

    /// Returns the children VariantValue's of a VariantValue::Ptr
    ChildMapType const getChildren() const;

    VariantValue::Ptr getChild(ValueId const valueId) const;

    /// Returns the first child
    VariantValue::Ptr getFirst() const;

    boost::asio::io_context::strand& getStrand();

protected:
    VariantValue() = delete;
    VariantValue(VariantValue const&&) = delete;
    VariantValue(VariantValue const&) = delete;

    virtual void signal(VariantValue::Ptr);
    void signal();

    void setUpdateType(SetUpdateType const);

    // Set Parent/Child relationships is the responsibility of the VariantValueStore

    /// Sets the parent VariantValue
    void parent(VariantValue::Ptr parent);

    /*!
     * Adds a child element to a VariantValue
     * Allows iterating over child values
     *
     * @param valuePtr  VariantValue::Ptr to add as a child
     */
    void addChild(VariantValue::Ptr valuePtr);

    /*!
     * Removes a child element from the VariantValue
     * @param   valueId leaf name of the child
     */
    void delChild(ValueId const& valueId);

    /*!
     * ParentSignal
     *
     * Used to call the parent from a child that the child has changed
     * This will in turn call the signal function
     */
    void parentSignal(VariantValue::Ptr const);

    /*!
     * Disconnects from all bound elements, signals, and shared_ptr's
     */
    void disconnect();

    /// Connects the WS change notification signal function for direct callback
    void setWsChangedSignal(ValueSignal::SlotType const&);

    friend class VariantValueStore;

    ChildMapType children_;

private:
    boost::asio::io_context::strand& ctx_;
    ValueSignal signal_;
    CompanEdgeProtocolValueData value_;
    ValueIdPtr valueId_; // The Value Store bucketizer has a copy of this

    // used to bypass signal -> signal delays
    ValueSignal::SlotType wsChangeNotification_;

    SetUpdateType setUpdateType_;

    VariantValue::Ptr parent_;

    VariantValueDispatcherPtr dataDispatcher_;
};

inline boost::asio::io_context::strand& VariantValue::getStrand()
{
    return ctx_;
}

inline void VariantValue::setDataDispatcher(VariantValueDispatcherPtr dataDispatcher)
{
    dataDispatcher_ = dataDispatcher;
}

inline CompanEdgeProtocol::Value_Type VariantValue::type() const
{
    return value_.type();
}

inline uint64_t VariantValue::hashToken() const
{
    return value_.hashToken();
}

inline void VariantValue::hashToken(uint64_t const& arg)
{
    value_.hashToken(arg);
}

inline CompanEdgeProtocol::Value_Access VariantValue::access() const
{
    return value_.access();
}

inline void VariantValue::access(CompanEdgeProtocol::Value_Access const arg)
{
    value_.access(arg);
}

inline VariantValue::SetUpdateType VariantValue::setUpdateType() const
{
    return setUpdateType_;
}

inline void VariantValue::setUpdateType(SetUpdateType const arg)
{
    setUpdateType_ = arg;
}

inline void VariantValue::parent(VariantValue::Ptr parent)
{
    parent_ = parent;
}
inline VariantValue::Ptr VariantValue::parent()
{
    return parent_;
}

inline SignalConnection VariantValue::connectChangedListener(ValueSignal::SlotType const& cb)
{
    return signal_.connect(cb);
}

inline void VariantValue::signal()
{
    signal(shared_from_this());
}

inline void VariantValue::parentSignal(VariantValue::Ptr const)
{
    signal();
}

inline void VariantValue::setWsChangedSignal(ValueSignal::SlotType const& cb)
{
    wsChangeNotification_ = cb;
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_VALUESTORE_VARIANT_H__
