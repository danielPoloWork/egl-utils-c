/*
 * d4np-c — fixed-block pool allocator (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/mem/pool.h"

#include <assert.h>
#include <stdalign.h>

typedef union d4np_pool_max_align {
    long double ld;
    long long ll;
    void *p;
    void (*fp)(void);
} d4np_pool_max_align_t;

static size_t round_up(size_t value, size_t multiple)
{
    /* `multiple` is a power of two (an alignment). */
    return (value + (multiple - 1)) & ~(multiple - 1);
}

d4np_status_t d4np_pool_init(d4np_pool_t *pool, const d4np_allocator_t *allocator, size_t block_size,
                             size_t block_count)
{
    if (pool == NULL || block_size == 0 || block_count == 0) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (allocator == NULL) {
        allocator = d4np_allocator_default();
    }

    const size_t align = alignof(d4np_pool_max_align_t);

    /* Each slot must hold a free-list pointer and be max-aligned. */
    size_t slot = block_size < sizeof(void *) ? sizeof(void *) : block_size;
    size_t stride = round_up(slot, align);
    if (stride < slot) {
        return D4NP_ERR_OVERFLOW;
    }
    /* total = stride * block_count, guarded against overflow. */
    if (stride != 0 && block_count > (size_t)-1 / stride) {
        return D4NP_ERR_OVERFLOW;
    }
    size_t total = stride * block_count;

    void *block = d4np_alloc(allocator, total, align);
    if (block == NULL) {
        return D4NP_ERR_OUT_OF_MEMORY;
    }

    pool->allocator = allocator;
    pool->base = (unsigned char *)block;
    pool->stride = stride;
    pool->block_count = block_count;
    pool->total_size = total;
    pool->free_count = block_count;

    /* Thread the free-list: each slot points at the next; the last points at NULL. */
    void *head = NULL;
    for (size_t i = block_count; i-- > 0;) {
        void *node = pool->base + (i * stride);
        *(void **)node = head;
        head = node;
    }
    pool->free_list = head;
    return D4NP_OK;
}

void *d4np_pool_alloc(d4np_pool_t *pool)
{
    if (pool == NULL || pool->free_list == NULL) {
        return NULL;
    }
    void *node = pool->free_list;
    pool->free_list = *(void **)node;
    pool->free_count--;
    return node;
}

void d4np_pool_free(d4np_pool_t *pool, void *ptr)
{
    if (pool == NULL || ptr == NULL) {
        return;
    }
    /* Debug-only provenance check: the pointer must lie on a slot boundary in the block. */
    assert((unsigned char *)ptr >= pool->base);
    assert((unsigned char *)ptr < pool->base + pool->total_size);
    assert((size_t)((unsigned char *)ptr - pool->base) % pool->stride == 0);

    *(void **)ptr = pool->free_list;
    pool->free_list = ptr;
    pool->free_count++;
}

void d4np_pool_destroy(d4np_pool_t *pool)
{
    if (pool == NULL || pool->base == NULL) {
        return;
    }
    d4np_free(pool->allocator, pool->base, pool->total_size);
    pool->allocator = NULL;
    pool->base = NULL;
    pool->free_list = NULL;
    pool->stride = 0;
    pool->block_count = 0;
    pool->total_size = 0;
    pool->free_count = 0;
}

size_t d4np_pool_available(const d4np_pool_t *pool)
{
    return (pool != NULL) ? pool->free_count : 0;
}
