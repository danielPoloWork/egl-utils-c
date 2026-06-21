/**
 * @file hashmap.h
 * @brief Generic hash map (open addressing, linear probing).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A cache-friendly hash map over fixed-size keys and values (sizes set at init, so one
 * implementation serves any POD key/value). Collisions are resolved by linear probing in an
 * open-addressed table whose capacity is a power of two; control bytes are kept in a separate
 * array so a probe scans contiguous memory. Keys and values are copied by value into
 * max-aligned slot arrays; the map never owns pointers the keys/values may contain (spec \#7).
 *
 * Hashing and key-equality are pluggable; passing NULL selects the defaults (FNV-1a over the
 * key bytes, and byte-wise equality).
 *
 * @ingroup d4np_ds
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

/**
 * @addtogroup d4np_ds
 * @{
 */

/** @brief Hash function over the raw key bytes; NULL selects the default (FNV-1a). */
typedef uint64_t (*d4np_hash_fn)(const void *key, size_t key_size);
/** @brief Key-equality predicate over the raw key bytes; NULL selects byte-wise equality. */
typedef bool (*d4np_key_eq_fn)(const void *a, const void *b, size_t key_size);

/** @brief An open-addressed hash map over fixed-size keys and values. */
typedef struct d4np_hashmap {
    const d4np_allocator_t *allocator; /**< backing allocator the storage came from   */
    uint8_t *control;                  /**< per-slot state: empty / occupied / deleted */
    unsigned char *keys;               /**< capacity * key_stride, max-aligned slots    */
    unsigned char *values;             /**< capacity * value_stride, max-aligned slots  */
    size_t key_size;                   /**< bytes per key (fixed at init)               */
    size_t value_size;                 /**< bytes per value (fixed at init)             */
    size_t key_stride;                 /**< per-slot key span including alignment        */
    size_t value_stride;               /**< per-slot value span including alignment      */
    size_t capacity;                   /**< power of two (0 before first insert)         */
    size_t len;                        /**< occupied slots                              */
    size_t tombstones;                 /**< deleted slots awaiting reclamation          */
    d4np_hash_fn hash;                 /**< active hash function                         */
    d4np_key_eq_fn key_eq;             /**< active key-equality predicate                */
} d4np_hashmap_t;

/**
 * @brief Initialize a hash map for keys and values of fixed byte sizes.
 *
 * @param m                Map to initialize.
 * @param allocator        Backing allocator, or NULL to select the default allocator.
 * @param key_size         Bytes per key; must be greater than 0.
 * @param value_size       Bytes per value; must be greater than 0.
 * @param initial_capacity Capacity hint, rounded up to a power of two; 0 means lazy allocation.
 * @param hash             Hash function, or NULL to use the default (FNV-1a over the key bytes).
 * @param key_eq           Key-equality predicate, or NULL to use byte-wise equality.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INVALID_ARGUMENT, ::D4NP_ERR_OVERFLOW, or
 *         ::D4NP_ERR_OUT_OF_MEMORY on failure.
 */
d4np_status_t d4np_hashmap_init(d4np_hashmap_t *m, const d4np_allocator_t *allocator, size_t key_size,
                                size_t value_size, size_t initial_capacity, d4np_hash_fn hash, d4np_key_eq_fn key_eq);

/**
 * @brief Release all storage and zero the map.
 * @param m Map to destroy.
 * @note Safe on a zeroed map.
 */
void d4np_hashmap_destroy(d4np_hashmap_t *m);

/**
 * @brief Insert or overwrite the value for a key (both copied in).
 * @param m     Map to insert into.
 * @param key   Pointer to @c key_size key bytes.
 * @param value Pointer to @c value_size value bytes.
 * @return ::D4NP_OK on success, or an error status on failure.
 * @note Amortized O(1).
 */
d4np_status_t d4np_hashmap_put(d4np_hashmap_t *m, const void *key, const void *value);

/**
 * @brief Pointer to the stored value for a key (mutable, in place).
 * @param m   Map to query.
 * @param key Pointer to @c key_size key bytes.
 * @return Pointer to the stored value, or NULL if the key is absent.
 */
void *d4np_hashmap_get(const d4np_hashmap_t *m, const void *key);

/**
 * @brief Whether a key is present.
 * @param m   Map to query.
 * @param key Pointer to @c key_size key bytes.
 * @return true if the key is present, false otherwise.
 */
bool d4np_hashmap_contains(const d4np_hashmap_t *m, const void *key);

/**
 * @brief Remove a key.
 * @param m   Map to remove from.
 * @param key Pointer to @c key_size key bytes.
 * @return true if the key was present, false otherwise.
 */
bool d4np_hashmap_remove(d4np_hashmap_t *m, const void *key);

/** @brief Number of entries stored. */
size_t d4np_hashmap_len(const d4np_hashmap_t *m);

/**
 * @brief Remove all entries, keeping the allocated capacity.
 * @param m Map to clear.
 */
void d4np_hashmap_clear(d4np_hashmap_t *m);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_DS_HASHMAP_H */
