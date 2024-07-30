/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_hash_token.cpp
 @brief Hash token containing two parts of a hash'd element
 */

#include "company_ref_variant_valuestore_hash_token.h"

#include <utility>

using namespace Compan::Edge;

HashToken::HashToken()
    : HashToken(InvalidBucket, InvalidHashId)
{
}

HashToken::HashToken(BucketType const b, HashIdType const i)
    : bucket_(b)
    , id_(i)
{
}

HashToken::HashToken(TransportType const arg)
    : HashToken(static_cast<BucketType>(arg >> 32), static_cast<HashIdType>(arg))
{
}

HashToken::HashToken(HashToken const& arg)
    : bucket_(arg.bucket_)
    , id_(arg.id_)
{
}

HashToken::HashToken(HashToken&& arg)
    : HashToken()
{
    std::swap(bucket_, arg.bucket_);
    std::swap(id_, arg.id_);
}

HashToken& HashToken::operator=(HashToken&& arg)
{
    std::swap(bucket_, arg.bucket_);
    std::swap(id_, arg.id_);

    return *this;
}

HashToken& HashToken::operator=(HashToken const& arg)
{
    bucket_ = arg.bucket_;
    id_ = arg.id_;

    return *this;
}

/*
 addToBucket and removeFromBucket are doing some bit manipulation to return a unique bucket identifier

 The add functionality, when there are no missing bits simple counts up:
    00000001
    00000011
    00000111
    00001111
    00011111
    00111111
    01111111

 Removal is a rare case in our library. It can be caused by a map value removing elements, while this is
 common, this is not an every second occurrence. The bucket identifier will start having bit holes in the
 sequence, for instance: 01101101

 The addToBucket figures out the missing portion, and returns a bucket identifier from the missing point:
    arg 	= 01101101 <- Start Point
    returns 00000011 <- Added
    arg now is 01101111

    arg 	= 01101111 <- Start Point
    returns 00011111 <- Added
    arg now is 01111111
*/

HashToken::BucketType HashToken::addToBucket(BucketType& arg)
{
    if (arg == 0) {
        arg = 1;
        return arg;
    }

    // if we've removed a value (rare event) accept this branch
    if ((32 - __builtin_clz(arg)) != __builtin_popcount(arg)) {
        // finds the trailing zero's after an xor operation:
        // 01101101 ^ 0xffffffff
        // = 10010010
        // Trailing zero count is 0x01
        uint32_t n = __builtin_ctz(arg ^ 0xffffffff);

        // Now we know what to shift 0xffffffff to give a correct result
        uint32_t shift = 31 - n;

        // Take the bit
        arg |= (1 << (n));

        // return the bucket identifier
        return (0xffffffff >> shift);
    }

    // Figure out the left most zero bit and set it to 1
    arg |= (1 << (32 - __builtin_clz(arg)));

    return arg;
}

void HashToken::removeFromBucket(BucketType& arg, BucketType const bucket)
{
    if (bucket == 0) return;

    // yes, this is embarassingly simple code, I know. Don't judge
    int bit = 32 - __builtin_clz(bucket);
    arg &= ~(1 << (bit - 1));
}
