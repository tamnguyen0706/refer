/**
  Copyright © 2023 COMPAN REF
  @file company_ref_variant_valuestore_value_id_bucketizer.h
  @brief Value id - bucketizer
*/

#ifndef __company_ref_VARIANT_VALUESTORE_VALUE_ID_BUCKETIZER_H__
#define __company_ref_VARIANT_VALUESTORE_VALUE_ID_BUCKETIZER_H__

#include "company_ref_variant_valuestore_hash_bucket.h"
#include "company_ref_variant_valuestore_hashtoken_map.h"
#include "company_ref_variant_valuestore_valueid.h"

#include <memory>

namespace Compan{
namespace Edge {

/*!
 * @brief Container for HashToken to Value names mappings
 *
 * Keeps a master registry for HashToken values associated with Value names.
 *
 * This container guarantees that there will never be a hash token collision with
 * a Value name.
 *
 * The hash identification is split into two portions, a bucket identifier and the string's hash result. Both values
 * are 32bit, making the total identifier a 64bit value.
 *
 * On a 32bit platform, hashing a 64bit value requires mathematical expressions that convert a 32bit value into a 64bit,
 * which in essence slows the process down. It is easier to hash the string to 32bit and use a 32bit bucket identifier
 * for clashed hash results.
 *
 * When searching for a HashToken type, it is as simple as looking by bucket, then id.
 *
 * In the case of a string search, the function would turn the string into a hash id, and walk through N number of
 * buckets to find the matching hash id, and physically compare the string of the value's id to the string being
 * searched for.
 *
 */
class ValueIdBucketizer {
public:
    ValueIdBucketizer();
    ~ValueIdBucketizer() = default;

    /// Returns the internal HashBucket container
    HashBucket& hashBucket();

    /*!
     * Returns a HashToken for a Value Id name.
     *
     * - If the Value Id name already exists, returns the pre-existing HashToken.
     * - If the Value Id name does not exist, creates and returns a new HashToken.
     *
     * @param Value Id String
     * @return  Valid HashToken
     */
    ///
    HashToken make(ValueId const&);

    /*!
     * Returns a Value Id that is associated with a HashToken
     * @param HashToken to search for
     * @return Valid ValueId::Ptr or nullptr
     */
    ValueId::Ptr get(HashToken const&) const;

    /*!
     * Removes a HashToken and associated Value Id name from the container
     * @param HashToken to remove
     */
    void removeToken(HashToken const&);

    /// Clears the hash bucketizer
    void clear();

    /// Return the size of the bucketizer elements
    size_t size() const;

private:
    HashToken::BucketType findById(HashToken::HashIdType const&, ValueId const&) const;

private:
    HashBucket hashBucket_;
    HashTokenMap<ValueId::Ptr> bucketStore_;
};

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_VARIANT_VALUESTORE_VALUE_ID_BUCKETIZER_H__*/
