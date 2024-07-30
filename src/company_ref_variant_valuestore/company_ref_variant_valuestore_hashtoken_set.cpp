/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_hashtoken_set.cpp
 @brief Simple Set of HashTokens
 */
#include "company_ref_variant_valuestore_hashtoken_set.h"

using namespace Compan::Edge;

HashTokenSet::HashTokenSet()
    : bucketMap_()
{
}

bool HashTokenSet::has(HashToken const& hashToken)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (bucketMap_.count(hashToken.bucket()) == 0) return false;

    return bucketMap_[hashToken.bucket()].count(hashToken.id());
}

bool HashTokenSet::insert(HashToken const& hashToken)
{
    if (!hashToken.valid()) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = bucketMap_[hashToken.bucket()].emplace(hashToken.id());
    return iter.second;
}

void HashTokenSet::remove(HashToken const& hashToken)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (bucketMap_.count(hashToken.bucket()) == 0) return;

    bucketMap_[hashToken.bucket()].erase(hashToken.id());
    if (bucketMap_[hashToken.bucket()].empty()) bucketMap_.erase(hashToken.bucket());
}

size_t HashTokenSet::size() const
{
    size_t numElements(0);

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& buckets : bucketMap_) numElements += buckets.second.size();

    return numElements;
}

bool HashTokenSet::empty() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return bucketMap_.empty();
}
