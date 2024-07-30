/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_hashtoken_set.h
 @brief Simple Set of HashTokens
 */
#ifndef __company_ref_VARIANT_VALUESTORE_HASHTOKEN_SET_H__
#define __company_ref_VARIANT_VALUESTORE_HASHTOKEN_SET_H__

#include "company_ref_variant_valuestore_hash_token.h"

#include <cstddef>
#include <unordered_map>
#include <unordered_set>

#include <mutex>

namespace Compan{
namespace Edge {

/*
 * @brief
 */
class HashTokenSet {
public:
    HashTokenSet();
    virtual ~HashTokenSet() = default;

    bool has(HashToken const& hashToken);
    bool insert(HashToken const& hashToken);
    void remove(HashToken const& hashToken);

    size_t size() const;

    bool empty() const;

private:
    typedef std::unordered_set<HashToken::HashIdType> HashIdSet;
    std::unordered_map<HashToken::BucketType, HashIdSet> bucketMap_;

    mutable std::mutex mutex_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_VALUESTORE_HASHTOKEN_SET_H__
