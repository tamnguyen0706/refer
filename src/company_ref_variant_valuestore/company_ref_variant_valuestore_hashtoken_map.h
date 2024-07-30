/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_hashtoken_map.h
 @brief Simple Set of HashTokens
 */
#ifndef __company_ref_VARIANT_VALUESTORE_HASHTOKEN_MAP_H__
#define __company_ref_VARIANT_VALUESTORE_HASHTOKEN_MAP_H__

#include "company_ref_variant_valuestore_hash_token.h"
#include <cstddef>
#include <mutex>
#include <unordered_map>

namespace Compan{
namespace Edge {

/*
 * @brief HashToken unordered map
 *
 * Simplistic interface to store associated data with a HashToken
 */
template <typename T>
class HashTokenMap {
public:
    using HashIdMap = std::unordered_map<HashToken::HashIdType, T>;
    using BucketMap = std::unordered_map<HashToken::BucketType, HashIdMap>;

    using HashIdIterator = typename HashIdMap::const_iterator;
    using BucketIterator = typename BucketMap::const_iterator;

public:
    HashTokenMap() = default;
    virtual ~HashTokenMap() = default;

    /*!
     * Checks if the HashToken is in the map
     * @param hashToken
     * @return True if found
     */
    bool has(HashToken const& hashToken) const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (bucketMap_.empty()) return false;

        auto bucketIter = bucketMap_.find(hashToken.bucket());
        if (bucketIter == bucketMap_.end()) return false;

        if (bucketIter->second.empty()) return false;

        auto idIter = bucketIter->second.find(hashToken.id());
        if (idIter == bucketIter->second.end()) return false;

        return true;
    }

    /*!
     * Inserts an element in the map
     *
     * @param hashToken
     * @param element
     * @return True on success
     */
    bool insert(HashToken const& hashToken, T const& element)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto iter = bucketMap_[hashToken.bucket()].insert(std::make_pair(hashToken.id(), element));
        return iter.second;
    }

    /*!
     * Inserts an element in the map
     *
     * @param hashToken
     * @param element
     * @return True on success
     */
    bool insert(HashToken const& hashToken, T&& element)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto iter = bucketMap_[hashToken.bucket()].insert(std::make_pair(hashToken.id(), std::move(element)));
        return iter.second;
    }

    /*!
     * Removes a element from the map
     * @param hashToken
     */
    void remove(HashToken const& hashToken)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (bucketMap_.count(hashToken.bucket()) == 0) return;

        bucketMap_[hashToken.bucket()].erase(hashToken.id());
        if (bucketMap_[hashToken.bucket()].empty()) bucketMap_.erase(hashToken.bucket());
    }

    /*!
     * Access or insert specified element
     *
     * @param hashToken
     *
     * @return Reference to the element
     */
    T& operator[](HashToken const& hashToken)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return bucketMap_[hashToken.bucket()][hashToken.id()];
    }

    /*!
     * Access specified element
     *
     * @param hashToken
     *
     * @return Reference to the element
     */
    T get(HashToken const& hashToken) const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto bucketIter = bucketMap_.find(hashToken.bucket());
        if (bucketIter == bucketMap_.end()) return T();

        auto hashIdIter = bucketIter->second.find(hashToken.id());
        if (hashIdIter == bucketIter->second.end()) return T();

        return hashIdIter->second;
    }

    /*!
     * Access or insert a HashID Map associated with a bucket identifier
     * @param hashToken
     * @return
     */
    HashIdMap& getHashIdMap(HashToken const& hashToken)
    {
        return getHashIdMap(hashToken.bucket());
    }

    /*!
     * Access or insert a HashID Map associated with a bucket identifier
     * @param bucketId
     * @return
     */
    HashIdMap& getHashIdMap(HashToken::BucketType const bucketId)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return bucketMap_[bucketId];
    }

    /*!
     * Returns the beginning bucket iterator
     * @return
     */
    BucketIterator begin() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return bucketMap_.begin();
    }

    /*!
     * Returns the ending bucket iterator
     * @return
     */
    BucketIterator end() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return bucketMap_.end();
    }

    /*!
     * Returns the beginning HashId map iterator
     * @return
     */
    HashIdIterator hashIdBegin(HashToken const hashToken) const
    {
        return hashIdBegin(hashToken.bucket());
    }

    /*!
     * Returns the beginning HashId map iterator
     * @return
     */
    HashIdIterator hashIdBegin(HashToken::BucketType const bucketId) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto bucketIter = bucketMap_.find(bucketId);
        if (bucketIter == bucketMap_.end()) return HashIdIterator();

        return bucketIter->second.begin();
    }

    /*!
     * Returns the ending bucket iterator
     * @return
     */
    HashIdIterator hashIdEnd(HashToken const hashToken) const
    {
        return hashIdEnd(hashToken.bucket());
    }

    /*!
     * Returns the ending bucket iterator
     * @return
     */
    HashIdIterator hashIdEnd(HashToken::BucketType const bucketId) const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto bucketIter = bucketMap_.find(bucketId);
        if (bucketIter == bucketMap_.end()) return HashIdIterator();

        return bucketIter->second.end();
    }

    /// returns the size of all the elements of the container
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        size_t numElements(0);

        for (auto& buckets : bucketMap_) numElements += buckets.second.size();

        return numElements;
    }

    /// True on empty
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return bucketMap_.empty();
    }

    void clear()
    {
        bucketMap_.clear();
    }

private:
    mutable std::mutex mutex_;
    BucketMap bucketMap_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_VALUESTORE_HASHTOKEN_MAP_H__
