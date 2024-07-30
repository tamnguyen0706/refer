/**
  Copyright © 2023 COMPAN REF
  @file company_ref_variant_valuestore_value_id_bucketizer.cpp
  @brief Value id - bucketizer
*/

#include "company_ref_variant_valuestore_value_id_bucketizer.h"

using namespace Compan::Edge;

ValueIdBucketizer::ValueIdBucketizer()
{
}

HashBucket& ValueIdBucketizer::hashBucket()
{
    return hashBucket_;
}

HashToken ValueIdBucketizer::make(ValueId const& valueId)
{
    HashToken::HashIdType hashId = hashBucket_.getHashFunction()(valueId);

    if (hashBucket_.hasBucket(hashId)) {
        HashToken::BucketType bucket = findById(hashId, valueId);
        if (bucket != HashToken::InvalidBucket) return HashToken(bucket, hashId);
    }

    HashToken token(hashBucket_.createToken(hashId));

    if (!bucketStore_.insert(token, std::make_shared<ValueId>(valueId))) { return HashToken(); }

    return token;
}

ValueId::Ptr ValueIdBucketizer::get(HashToken const& hashToken) const
{
    return bucketStore_.get(hashToken);
}

void ValueIdBucketizer::removeToken(HashToken const& hashToken)
{
    hashBucket_.removeToken(hashToken);
    bucketStore_.remove(hashToken);
}

HashToken::BucketType ValueIdBucketizer::findById(HashToken::HashIdType const& id, ValueId const& valueId) const
{
    for (auto& bucketsIter : bucketStore_) {

        for (auto hashIter = bucketStore_.hashIdBegin(bucketsIter.first);
             hashIter != bucketStore_.hashIdEnd(bucketsIter.first);
             ++hashIter) {

            if (hashIter->first == id && *(hashIter->second) == valueId) return bucketsIter.first;
        }
    }
    return HashToken::InvalidBucket;
}

void ValueIdBucketizer::clear()
{
    hashBucket_.reset();
    bucketStore_.clear();
}

size_t ValueIdBucketizer::size() const
{
    return bucketStore_.size();
}
