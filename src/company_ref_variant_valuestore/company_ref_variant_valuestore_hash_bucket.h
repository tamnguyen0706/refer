/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_hash_bucket.h
 @brief Container for Hash'd elements using a Bit bucket identifier and a hash value. Stores only uint32_t elements
 */

#ifndef __company_ref_VARIANT_VALUESTORE_HASH_BUCKET_H__
#define __company_ref_VARIANT_VALUESTORE_HASH_BUCKET_H__

#include "company_ref_variant_valuestore_hash_token.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

namespace Compan{
namespace Edge {

class HashBucket {
public:
    typedef std::function<uint32_t(std::string const&)> HashFunction;

    HashBucket();
    virtual ~HashBucket();

    /// used to reset the hash bucket
    void reset();

    /// used to change from the default hash function
    void setHashFunction(HashFunction hf);
    HashFunction getHashFunction() const;

    HashToken createToken(std::string const& arg);
    HashToken createToken(HashToken::HashIdType const& hashId);

    void removeToken(HashToken const& token);

    /// checks for the presence of a hash id - no hash, no bucket
    bool hasBucket(HashToken::HashIdType const&);

    /// returns the bucket value for a hash id
    HashToken::BucketType getBucket(HashToken::HashIdType const&);

    /// Total number of hashed elements
    size_t size() const;

    /// Total number of collisions elements, spread across buckets
    size_t collisions() const;

    /// Max number of buckets
    size_t buckets() const;

    /// Total number of errors relating to running out of hash or bucket id's
    size_t errors() const;

    std::string getStats() const;

private:
    HashFunction hf_;

    size_t size_;
    size_t collisions_;
    size_t buckets_;
    size_t errors_;

    std::unordered_map<HashToken::HashIdType, HashToken::BucketType> hashBucket_;
};

inline void HashBucket::setHashFunction(HashFunction hf)
{
    hf_ = std::move(hf);
}

inline HashBucket::HashFunction HashBucket::getHashFunction() const
{
    return hf_;
}

inline size_t HashBucket::size() const
{
    return size_;
}

inline size_t HashBucket::collisions() const
{
    return collisions_;
}

inline size_t HashBucket::buckets() const
{
    return buckets_;
}

inline size_t HashBucket::errors() const
{
    return errors_;
}

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_VARIANT_VALUESTORE_HASH_BUCKET_H__*/
