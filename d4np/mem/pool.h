/*
 * d4np-c — fixed-block pool allocator.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A pool pre-allocates one block holding `block_count` fixed-size slots and threads the free
 * ones into an intrusive singly-linked free-list. Allocation and free are O(1) pointer swaps
 * with no per-object headers and no fragmentation — ideal for many same-sized objects with
 * churn (spec #4–#5). The free-list pointer is stored inside each free slot, so the effective
 * slot stride is at least sizeof(void*), rounded up for alignment.
 */
#ifndef D4NP_MEM_POOL_H
#define D4NP_MEM_POOL_H

#include <stddef.h>

#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct d4np_pool {
    const d4np_allocator_t *allocator; /* backing allocator the block came from */
    unsigned char *base;               /* start of the backing block            */
    void *free_list;                   /* head of the intrusive free-list        */
    size_t stride;                     /* bytes between slots (>= block_size)     */
    size_t block_count;                /* number of slots                        */
    size_t total_size;                 /* base block size = stride * block_count  */
    size_t free_count;                 /* slots currently free                   */
} d4np_pool_t;

/*
 * Initialize `pool` with `block_count` slots of at least `block_size` bytes (rounded up to hold
 * a free-list pointer and to the platform's max alignment), drawn from `allocator` (NULL ->
 * default). `block_size` and `block_count` must be > 0.
 * Returns D4NP_OK, D4NP_ERR_INVALID_ARGUMENT, D4NP_ERR_OVERFLOW (size computation), or
 * D4NP_ERR_OUT_OF_MEMORY.
 */
d4np_status_t d4np_pool_init(d4np_pool_t *pool, const d4np_allocator_t *allocator, size_t block_size,
                             size_t block_count);

/* O(1) allocation of one slot, or NULL when the pool is exhausted. Memory is uninitialized. */
void *d4np_pool_alloc(d4np_pool_t *pool);

/*
 * O(1) return of a slot previously handed out by `d4np_pool_alloc` on this pool. Passing NULL
 * is a no-op. Passing a pointer not owned by this pool is undefined behavior (the caller owns
 * provenance), though debug builds assert the pointer lies within the block.
 */
void d4np_pool_free(d4np_pool_t *pool, void *ptr);

/* Return the backing block to the allocator and zero the pool. Safe on a zeroed pool. */
void d4np_pool_destroy(d4np_pool_t *pool);

/* Slots currently available. */
size_t d4np_pool_available(const d4np_pool_t *pool);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_MEM_POOL_H */
