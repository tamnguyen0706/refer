/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_hash_bucket.cpp
 @brief Container for Hash'd elements using a Bit bucket identifier and a hash value. Stores only uint32_t elements
 */

#include "company_ref_variant_valuestore_hash_bucket.h"

#include "company_ref_variant_valuestore_hash_methods.h"
#include "company_ref_variant_valuestore_hash_token.h"
#include <sstream>

using namespace Compan::Edge;

HashBucket::HashBucket()
    : size_(0)
    , collisions_(0)
    , buckets_(0)
    , errors_(0)
{
    // our default hasher, since we're running mostly on a MIPS. Seems to perform best
    setHashFunction(&FnvHash::as32);
}

HashBucket::~HashBucket()
{
}

void HashBucket::reset()
{
    size_ = 0;
    collisions_ = 0;
    buckets_ = 0;
    errors_ = 0;
    hashBucket_.clear();
}

HashToken HashBucket::createToken(std::string const& arg)
{
    HashToken::HashIdType id(hf_(arg));

    // this is nearly impossible, if it DOES happen, probably should glitch
    if (id == HashToken::InvalidHashId) {
        ++errors_;
        return HashToken();
    }

    return createToken(id);
}

HashToken HashBucket::createToken(HashToken::HashIdType const& hashId)
{
    if (hashId == HashToken::InvalidHashId) { return HashToken(); }

    // if the id doesn't exist, throw it in the any bucket ... we are good
    if (!hashBucket_.count(hashId)) {
        hashBucket_[hashId] = 0;

        ++size_;
        return HashToken(0, hashId);
    }

    // manipulate the bucket
    HashToken::BucketType& bucketVal = hashBucket_.at(hashId);

    HashToken::BucketType bucket = HashToken::addToBucket(bucketVal);
    if (bucket == HashToken::InvalidBucket) {
        ++errors_;
        return HashToken();
    }

    ++collisions_;
    buckets_ = std::max(buckets_, static_cast<size_t>(__builtin_popcount(bucketVal)));
    ++size_;

    return HashToken(bucket, hashId);
}

void HashBucket::removeToken(HashToken const& token)
{
    if (!token.valid()) return;

    if (!hashBucket_.count(token.id())) return;

    HashToken::BucketType& bucketVal = hashBucket_.at(token.id());
    HashToken::removeFromBucket(bucketVal, token.bucket());
    --size_;

    if (bucketVal == 0) hashBucket_.erase(token.id());
}

bool HashBucket::hasBucket(HashToken::HashIdType const& id)
{
    return hashBucket_.count(id);
}

HashToken::BucketType HashBucket::getBucket(HashToken::HashIdType const& id)
{
    if (!hashBucket_.count(id)) return HashToken::InvalidBucket;

    return hashBucket_.at(id);
}

std::string HashBucket::getStats() const
{
    std::stringstream strm;
    strm << " Size:" << size_ << " = "
         << "Elements: " << hashBucket_.size() << " + "
         << " Collisions: " << collisions_ << " [" << buckets_ << "]" << std::endl;

    return strm.str();
}
