/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_valuedata.h
 @brief CompanEdgeProtocol::Value's data encapsulation
 */
#ifndef __company_ref_VARIANT_VALUESTORE_VALUEDATA_H__
#define __company_ref_VARIANT_VALUESTORE_VALUEDATA_H__

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <boost/core/noncopyable.hpp>

namespace Compan{
namespace Edge {

struct ValueDataSet {

    enum Results {
        Success = 0, //!< Successful set of a value
        SameValue,   //!< Warning - the value is the same
        InvalidType, //!< Error - type isn't correct
        RangeError,  //!< Error - Integral out of range
        EnumError,   //!< Error - Enum string or value is out of range
        AccessError, //!< Error - Writing to a ReadOnly value
    };

    static std::string resultStr(Results const arg);
};

/*!
 * CompanEdgeProtocolValueData is a containment class to
 * protect access to the CompanEdgeProtocol::Value
 *
 * This class is a utility class intended for the
 * CompanEdgeProtocolValueData class, to guarantee future developers
 * do NOT use the value directly, given the asynchronous
 * nature of get/set
 *
 * This class is a flyweight
 */
class CompanEdgeProtocolValueData : private boost::noncopyable {
public:
    explicit CompanEdgeProtocolValueData(CompanEdgeProtocol::Value const& data);
    CompanEdgeProtocolValueData(CompanEdgeProtocol::Value_Type const type, CompanEdgeProtocol::Value_Access const access);

    virtual ~CompanEdgeProtocolValueData() = default;

    /// Returns the CompanEdgeProtocol::Value::type
    CompanEdgeProtocol::Value_Type type() const;

    /// Sets the CompanEdgeProtocol::Value::type
    void type(CompanEdgeProtocol::Value_Type const&);

    /// Returns the CompanEdgeProtocol::Value::hashtoken
    uint64_t hashToken() const;

    /// Sets the CompanEdgeProtocol::Value::hashtoken
    void hashToken(uint64_t const&);

    /// Returns the CompanEdgeProtocol::Value::access
    CompanEdgeProtocol::Value_Access access() const;

    /// Sets the CompanEdgeProtocol::Value::access
    void access(CompanEdgeProtocol::Value_Access const);

    /// Returns a copy of the CompanEdgeProtocol::Value object
    CompanEdgeProtocol::Value get() const;

    /*!
     * Attempts to set the under laying data from another CompanEdgeProtocol::Value object.
     *
     * @param arg           value information to set
     * @return Results value
     */
    ValueDataSet::Results set(CompanEdgeProtocol::Value const& arg);

    /// Returns true if data has been set to the CompanEdgeProtocol::Value object.
    bool hasData();

private:
    CompanEdgeProtocol::Value value_;
};

inline CompanEdgeProtocol::Value_Type CompanEdgeProtocolValueData::type() const
{
    return value_.type();
}
inline void CompanEdgeProtocolValueData::type(CompanEdgeProtocol::Value_Type const& arg)
{
    value_.set_type(arg);
}

inline uint64_t CompanEdgeProtocolValueData::hashToken() const
{
    return value_.hashtoken();
}

inline void CompanEdgeProtocolValueData::hashToken(uint64_t const& arg)
{
    value_.set_hashtoken(arg);
}

inline CompanEdgeProtocol::Value_Access CompanEdgeProtocolValueData::access() const
{
    return value_.access();
}

inline void CompanEdgeProtocolValueData::access(CompanEdgeProtocol::Value_Access const arg)
{
    value_.set_access(arg);
}
inline CompanEdgeProtocol::Value CompanEdgeProtocolValueData::get() const
{
    return value_;
}

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_VALUESTORE_VALUEDATA_H__
