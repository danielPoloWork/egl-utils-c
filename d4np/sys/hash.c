/*
 * d4np-c — FNV-1a hashing (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/sys/hash.h"

#include <string.h>

#define D4NP_FNV1A_PRIME 1099511628211ULL

uint64_t d4np_hash_fnv1a_continue(uint64_t seed, const void *data, size_t size)
{
    const unsigned char *p = (const unsigned char *)data;
    uint64_t h = seed;
    if (p != NULL) {
        for (size_t i = 0; i < size; ++i) {
            h ^= (uint64_t)p[i];
            h *= D4NP_FNV1A_PRIME;
        }
    }
    return h;
}

uint64_t d4np_hash_fnv1a(const void *data, size_t size)
{
    return d4np_hash_fnv1a_continue(D4NP_FNV1A_OFFSET_BASIS, data, size);
}

uint64_t d4np_hash_fnv1a_str(const char *cstr)
{
    return d4np_hash_fnv1a(cstr, (cstr != NULL) ? strlen(cstr) : 0);
}
