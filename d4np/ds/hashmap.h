/*
 * d4np-c — generic hash map (open addressing, linear probing).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A cache-friendly hash map over fixed-size keys and values (sizes set at init, so one
 * implementation serves any POD key/value). Collisions are resolved by linear probing in an
 * open-addressed table whose capacity is a power of two; control bytes are kept in a separate
 * array so a probe scans contiguous memory. Keys and values are copied by value into
 * max-aligned slot arrays; the map never owns pointers the keys/values may contain (spec #7).
 *
 * Hashing and key-equality are pluggable; passing NULL selects the defaults (FNV-1a over the
 * key bytes, and byte-wise equality).
 */
#ifndef D4NP_DS_HASHMAP_H
#define D4NP_DS_HASHMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t (*d4np_hash_fn)(const void *key, size_t key_size);
typedef bool (*d4np_key_eq_fn)(const void *a, const void *b, size_t key_size);

typedef struct d4np_hashmap {
    const d4np_allocator_t *allocator;
    uint8_t *control;      /* per-slot state: empty / occupied / deleted */
    unsigned char *keys;   /* capacity * key_stride, max-aligned slots    */
    unsigned char *values; /* capacity * value_stride, max-aligned slots  */
    size_t key_size;
    size_t value_size;
    size_t key_stride;
    size_t value_stride;
    size_t capacity;   /* power of two (0 before first insert)         */
    size_t len;        /* occupied slots                              */
    size_t tombstones; /* deleted slots awaiting reclamation          */
    d4np_hash_fn hash;
    d4np_key_eq_fn key_eq;
} d4np_hashmap_t;

/*
 * Initialize `m` for keys of `key_size` and values of `value_size` (both > 0), drawn from
 * `allocator` (NULL -> default). `initial_capacity` is a hint (rounded up to a power of two;
 * 0 = lazy). `hash`/`key_eq` may be NULL to use the defaults. Returns D4NP_OK,
 * D4NP_ERR_INVALID_ARGUMENT, D4NP_ERR_OVERFLOW, or D4NP_ERR_OUT_OF_MEMORY.
 */
d4np_status_t d4np_hashmap_init(d4np_hashmap_t *m, const d4np_allocator_t *allocator, size_t key_size,
                                size_t value_size, size_t initial_capacity, d4np_hash_fn hash, d4np_key_eq_fn key_eq);

/* Release all storage and zero the map. Safe on a zeroed map. */
void d4np_hashmap_destroy(d4np_hashmap_t *m);

/* Insert or overwrite the value for `key` (both copied in). Amortized O(1). */
d4np_status_t d4np_hashmap_put(d4np_hashmap_t *m, const void *key, const void *value);

/* Pointer to the stored value for `key` (mutable, in place), or NULL if absent. */
void *d4np_hashmap_get(const d4np_hashmap_t *m, const void *key);

/* Whether `key` is present. */
bool d4np_hashmap_contains(const d4np_hashmap_t *m, const void *key);

/* Remove `key`; returns true if it was present. */
bool d4np_hashmap_remove(d4np_hashmap_t *m, const void *key);

size_t d4np_hashmap_len(const d4np_hashmap_t *m);

/* Remove all entries, keeping the allocated capacity. */
void d4np_hashmap_clear(d4np_hashmap_t *m);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_DS_HASHMAP_H */
