/**
 * @file pool.h
 * @brief Fixed-block pool allocator.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A pool pre-allocates one block holding @c block_count fixed-size slots and threads the free
 * ones into an intrusive singly-linked free-list. Allocation and free are O(1) pointer swaps
 * with no per-object headers and no fragmentation — ideal for many same-sized objects with
 * churn (spec \#4–\#5). The free-list pointer is stored inside each free slot, so the effective
 * slot stride is at least sizeof(void*), rounded up for alignment.
 *
 * @ingroup d4np_mem
 */
#ifndef D4NP_MEM_POOL_H
#define D4NP_MEM_POOL_H

#include <stddef.h>

#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_mem
 * @{
 */

/** @brief A fixed-block pool over a single backing block with an intrusive free-list. */
typedef struct d4np_pool {
    const d4np_allocator_t *allocator; /**< backing allocator the block came from */
    unsigned char *base;               /**< start of the backing block            */
    void *free_list;                   /**< head of the intrusive free-list        */
    size_t stride;                     /**< bytes between slots (>= block_size)     */
    size_t block_count;                /**< number of slots                        */
    size_t total_size;                 /**< base block size = stride * block_count  */
    size_t free_count;                 /**< slots currently free                   */
} d4np_pool_t;

/**
 * @brief Initialize a pool with a freshly allocated backing block of fixed-size slots.
 *
 * Each slot is at least @p block_size bytes, rounded up to hold a free-list pointer and to the
 * platform's maximum alignment.
 *
 * @param pool        Pool to initialize (must be non-NULL).
 * @param allocator   Backing allocator, or NULL to select the default allocator.
 * @param block_size  Minimum slot size in bytes; must be greater than 0.
 * @param block_count Number of slots; must be greater than 0.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INVALID_ARGUMENT for invalid arguments;
 *         ::D4NP_ERR_OVERFLOW if the size computation overflows; ::D4NP_ERR_OUT_OF_MEMORY if
 *         the backing allocation fails.
 */
d4np_status_t d4np_pool_init(d4np_pool_t *pool, const d4np_allocator_t *allocator, size_t block_size,
                             size_t block_count);

/**
 * @brief O(1) allocation of one slot.
 *
 * @param pool Pool to allocate from.
 * @return A pointer to one slot, or NULL when the pool is exhausted. The memory is
 *         uninitialized.
 */
void *d4np_pool_alloc(d4np_pool_t *pool);

/**
 * @brief O(1) return of a slot previously handed out by d4np_pool_alloc() on this pool.
 *
 * @param pool Pool that owns the slot.
 * @param ptr  Slot to return; NULL is a no-op.
 * @note Passing a pointer not owned by this pool is undefined behavior (the caller owns
 *       provenance), though debug builds assert the pointer lies within the block.
 */
void d4np_pool_free(d4np_pool_t *pool, void *ptr);

/** @brief Return the backing block to the allocator and zero the pool. Safe on a zeroed pool. */
void d4np_pool_destroy(d4np_pool_t *pool);

/** @brief Slots currently available. */
size_t d4np_pool_available(const d4np_pool_t *pool);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_MEM_POOL_H */
