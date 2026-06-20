/*
 * d4np-c — arena (bump-pointer) allocator.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * An arena owns one contiguous backing block obtained from a d4np_allocator_t. Allocation is
 * an O(1) bump of an offset; individual frees are not supported. Instead the whole arena is
 * freed in one O(1) reset (keeping the block for reuse) or destroyed (returning the block to
 * the backing allocator). This removes per-allocation overhead and fragmentation on hot paths
 * with a clear lifetime (spec #1–#3).
 */
#ifndef D4NP_MEM_ARENA_H
#define D4NP_MEM_ARENA_H

#include <stddef.h>

#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct d4np_arena {
    const d4np_allocator_t *allocator; /* backing allocator the block came from */
    unsigned char *base;               /* start of the backing block            */
    size_t capacity;                   /* size of the backing block in bytes     */
    size_t offset;                     /* bytes handed out so far                */
} d4np_arena_t;

/*
 * Initialize `arena` with a freshly allocated block of `capacity` bytes drawn from
 * `allocator` (NULL selects the default allocator). `capacity` must be > 0.
 * Returns D4NP_OK, D4NP_ERR_INVALID_ARGUMENT (NULL arena / zero capacity), or
 * D4NP_ERR_OUT_OF_MEMORY.
 */
d4np_status_t d4np_arena_init(d4np_arena_t *arena, const d4np_allocator_t *allocator, size_t capacity);

/*
 * O(1) allocation of `size` bytes aligned to `align` (a power of two; 0 means byte alignment).
 * Returns a pointer within the arena, or NULL if `size` is 0, the request does not fit, or the
 * computation would overflow. Memory is uninitialized and valid until the next reset/destroy.
 */
void *d4np_arena_alloc(d4np_arena_t *arena, size_t size, size_t align);

/* O(1) bulk free: makes the whole capacity available again without releasing the block. */
void d4np_arena_reset(d4np_arena_t *arena);

/* Return the backing block to the allocator and zero the arena. Safe on a zeroed arena. */
void d4np_arena_destroy(d4np_arena_t *arena);

/* Bytes currently handed out, and bytes still available. */
size_t d4np_arena_used(const d4np_arena_t *arena);
size_t d4np_arena_remaining(const d4np_arena_t *arena);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_MEM_ARENA_H */
