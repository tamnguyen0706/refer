/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_hash_token.h
 @brief Hash token containing two parts of a hash'd element
 */

#ifndef __company_ref_VARIANT_VALUESTORE_HASH_TOKEN_H__
#define __company_ref_VARIANT_VALUESTORE_HASH_TOKEN_H__

#include <cstdint>
#include <limits>

namespace Compan{
namespace Edge {

/*!
 * @brief Bucket/Id Hash Token Pair
 *
 * Helper class for managing the two part hash value in a holistic manner
 *
 */
class HashToken {
public:
    typedef uint32_t HashIdType;    //!< 32bit Hash Value from FNV
    typedef uint32_t BucketType;    //!< Bucket identifier
    typedef uint64_t TransportType; //!< Used for transport

    constexpr static uint32_t InvalidHashId = std::numeric_limits<HashIdType>::max();
    constexpr static uint32_t InvalidBucket = std::numeric_limits<BucketType>::max();
    constexpr static uint64_t InvalidTokenType = std::numeric_limits<TransportType>::max();

    /// Constructs an invalid HashToken
    HashToken();

    /// Constructs a HashToken with the individual parts
    HashToken(BucketType const bucket, HashIdType const id);

    /// Constructs a HashToken from a transport token
    HashToken(TransportType const arg);

    /// Copy constructor
    HashToken(HashToken const&);

    /// Move constructor
    HashToken(HashToken&&);

    virtual ~HashToken() = default;

    /// Assign from another HashToken
    HashToken& operator=(HashToken&&);

    /// Assign from another HashToken
    HashToken& operator=(HashToken const&);

    /// Less than comparison
    bool operator<(HashToken const&) const;

    /// Not equal comparison
    bool operator==(HashToken const&) const;

    /// Not equal comparison
    bool operator!=(HashToken const&) const;

    /// Returns the bucket identifier
    BucketType bucket() const;

    /// Returns the FNV hash value
    HashIdType id() const;

    /// Returns the token transport value
    TransportType transportToken() const;

    /// Returns true if the bucket is 0
    bool isAnyBucket() const;

    /// Validates that the individual parts are valid
    bool valid() const;

    // Static functions for add/removal of bits to a bucket identifier
    static BucketType addToBucket(BucketType& arg);
    static void removeFromBucket(BucketType& arg, BucketType const bucket);

private:
    BucketType bucket_;
    HashIdType id_;
};

inline bool HashToken::operator<(HashToken const& token) const
{
    return (bucket_ || token.bucket_) ? id_ < token.id_ || bucket_ < token.bucket_ : id_ < token.id_;
}

inline bool HashToken::operator==(HashToken const& token) const
{
    return bucket_ == token.bucket_ && id_ == token.id_;
}

inline bool HashToken::operator!=(HashToken const& token) const
{
    return !(*this == token);
}

inline HashToken::BucketType HashToken::bucket() const
{
    return bucket_;
}

inline HashToken::HashIdType HashToken::id() const
{
    return id_;
}

inline HashToken::TransportType HashToken::transportToken() const
{
    return static_cast<TransportType>(bucket_) << 32 | id_;
}

inline bool HashToken::isAnyBucket() const
{
    return (bucket_ == 0);
}

inline bool HashToken::valid() const
{
    return (bucket_ != InvalidBucket && id_ != InvalidHashId);
}

} // namespace Edge
} // namespace Compan

inline bool operator<(COMPAN::REF::HashToken const& lhs, COMPAN::REF::HashToken const& rhs)
{
    return lhs.operator<(rhs);
}

#endif /*__company_ref_VARIANT_VALUESTORE_HASH_TOKEN_H__*/
