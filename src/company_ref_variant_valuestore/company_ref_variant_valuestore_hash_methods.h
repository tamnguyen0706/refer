/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_hash_methods.h
 @brief Common Hash functions for various libraries
 */

#ifndef __company_ref_VARIANT_VALUESTORE_HASH_METHODS_H__
#define __company_ref_VARIANT_VALUESTORE_HASH_METHODS_H__

#include <cstdint>
#include <string>

namespace Compan{
namespace Edge {

/*!
 * @brief Fowler–Noll–Vo hashing functions
 */
struct FnvHash {
    /// String to u16
    static uint16_t as16(std::string const& arg);
    /// String to u32
    static uint32_t as32(std::string const& arg);
    /// String to u64
    static uint64_t as64(std::string const& arg);
};

} // namespace Edge
} // namespace Compan

#endif /*__company_ref_VARIANT_VALUESTORE_HASH_METHODS_H__*/
