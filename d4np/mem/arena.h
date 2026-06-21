/**
 * @file arena.h
 * @brief Arena (bump-pointer) allocator.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * An arena owns one contiguous backing block obtained from a ::d4np_allocator_t. Allocation is
 * an O(1) bump of an offset; individual frees are not supported. Instead the whole arena is
 * freed in one O(1) reset (keeping the block for reuse) or destroyed (returning the block to
 * the backing allocator). This removes per-allocation overhead and fragmentation on hot paths
 * with a clear lifetime (spec \#1–\#3).
 *
 * @ingroup d4np_mem
 */
#ifndef D4NP_MEM_ARENA_H
#define D4NP_MEM_ARENA_H

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

/** @brief A bump-pointer arena over a single backing block. */
typedef struct d4np_arena {
    const d4np_allocator_t *allocator; /**< backing allocator the block came from */
    unsigned char *base;               /**< start of the backing block            */
    size_t capacity;                   /**< size of the backing block in bytes     */
    size_t offset;                     /**< bytes handed out so far                */
} d4np_arena_t;

/**
 * @brief Initialize an arena with a freshly allocated backing block.
 *
 * @param arena     Arena to initialize (must be non-NULL).
 * @param allocator Backing allocator, or NULL to select the default allocator.
 * @param capacity  Block size in bytes; must be greater than 0.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INVALID_ARGUMENT for a NULL arena or zero capacity;
 *         ::D4NP_ERR_OUT_OF_MEMORY if the backing allocation fails.
 */
d4np_status_t d4np_arena_init(d4np_arena_t *arena, const d4np_allocator_t *allocator, size_t capacity);

/**
 * @brief O(1) allocation of @p size bytes aligned to @p align.
 *
 * @param arena Arena to allocate from.
 * @param size  Number of bytes requested.
 * @param align Alignment, a power of two; 0 means byte alignment.
 * @return A pointer within the arena, or NULL if @p size is 0, the request does not fit, or the
 *         offset computation would overflow. The memory is uninitialized and stays valid until
 *         the next reset or destroy.
 */
void *d4np_arena_alloc(d4np_arena_t *arena, size_t size, size_t align);

/** @brief O(1) bulk free: make the whole capacity available again without releasing the block. */
void d4np_arena_reset(d4np_arena_t *arena);

/** @brief Return the backing block to the allocator and zero the arena. Safe on a zeroed arena. */
void d4np_arena_destroy(d4np_arena_t *arena);

/** @brief Bytes currently handed out. */
size_t d4np_arena_used(const d4np_arena_t *arena);

/** @brief Bytes still available for allocation. */
size_t d4np_arena_remaining(const d4np_arena_t *arena);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_MEM_ARENA_H */
