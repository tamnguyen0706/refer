/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_hash_methods.cpp
 @brief Common Hash functions for various libraries
 */

#include "company_ref_variant_valuestore_hash_methods.h"

namespace {

inline uint16_t Fold32to16(uint32_t const& arg)
{
    return (arg >> 16) ^ (arg & 0xffff);
}

/*!
 @brief FNV 1a hash algorithm
 @return  Hashed value in the bitspace provided by T
 @tparam  T   Type to return
 @tparam  Iterator    Iterator type to use
 */
template <typename T, typename Iterator>
T fnv1a_hash(
        Iterator const& begin, //!< Starting point for the hash range
        Iterator const& end,   //!< End point for the hash range
        T const prime,         //!< Prime value for calculate
        T const offset         //!< Starting exponential for hash calc
)
{
    T result(offset);
    for (Iterator it = begin; it != end; ++it) {
        result ^= *it;
        result *= prime;
    }

    return result;
}

} // namespace

using namespace Compan::Edge;

uint16_t FnvHash::as16(std::string const& arg)
{
    return Fold32to16(as32(arg));
}

uint32_t FnvHash::as32(std::string const& arg)
{
    return fnv1a_hash<uint32_t>(arg.begin(), arg.end(), 16777619u, 2166136261u);
}

uint64_t FnvHash::as64(std::string const& arg)
{
    return fnv1a_hash(arg.begin(), arg.end(), 1099511628211ull, 14695981039346656037ull);
}
