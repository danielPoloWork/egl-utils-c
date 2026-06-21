/*
 * d4np-c — FNV-1a hashing.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * The 64-bit Fowler-Noll-Vo (FNV-1a) hash over arbitrary bytes — small, fast, and good enough
 * for hash tables and checksums (not for cryptographic or adversarial use) (spec #25). A
 * continuation form lets a hash be computed incrementally over several chunks.
 */
#ifndef D4NP_SYS_HASH_H
#define D4NP_SYS_HASH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The FNV-1a 64-bit offset basis — the seed for a fresh hash. */
#define D4NP_FNV1A_OFFSET_BASIS 14695981039346656037ULL

/* Hash `size` bytes of `data`. A NULL/empty buffer hashes to the offset basis. */
uint64_t d4np_hash_fnv1a(const void *data, size_t size);

/* Convenience: hash a NUL-terminated string (excluding the terminator). */
uint64_t d4np_hash_fnv1a_str(const char *cstr);

/*
 * Continue a running hash: pass D4NP_FNV1A_OFFSET_BASIS for the first chunk and the previous
 * return value for each subsequent chunk. hash(a ++ b) == continue(continue(basis, a), b).
 */
uint64_t d4np_hash_fnv1a_continue(uint64_t seed, const void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_SYS_HASH_H */
