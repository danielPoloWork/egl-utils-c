/*
 * d4np-c — arena (bump-pointer) allocator (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/mem/arena.h"

#include <stdalign.h>

/* Widest fundamental alignment; the backing block is requested with it so any sub-alignment
 * up to this bound is satisfiable from within the arena. */
typedef union d4np_arena_max_align {
    long double ld;
    long long ll;
    void *p;
    void (*fp)(void);
} d4np_arena_max_align_t;

static size_t align_up(size_t value, size_t align, int *overflow)
{
    /* align is a power of two (or 0 -> 1). (value + align-1) & ~(align-1), with overflow guard. */
    size_t a = (align == 0) ? 1 : align;
    size_t sum = value + (a - 1);
    if (sum < value) {
        *overflow = 1;
        return 0;
    }
    return sum & ~(a - 1);
}

d4np_status_t d4np_arena_init(d4np_arena_t *arena, const d4np_allocator_t *allocator, size_t capacity)
{
    if (arena == NULL || capacity == 0) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (allocator == NULL) {
        allocator = d4np_allocator_default();
    }

    void *block = d4np_alloc(allocator, capacity, alignof(d4np_arena_max_align_t));
    if (block == NULL) {
        return D4NP_ERR_OUT_OF_MEMORY;
    }

    arena->allocator = allocator;
    arena->base = (unsigned char *)block;
    arena->capacity = capacity;
    arena->offset = 0;
    return D4NP_OK;
}

void *d4np_arena_alloc(d4np_arena_t *arena, size_t size, size_t align)
{
    if (arena == NULL || size == 0) {
        return NULL;
    }

    int overflow = 0;
    size_t aligned = align_up(arena->offset, align, &overflow);
    if (overflow || aligned > arena->capacity) {
        return NULL;
    }
    if (size > arena->capacity - aligned) {
        return NULL;
    }

    void *ptr = arena->base + aligned;
    arena->offset = aligned + size;
    return ptr;
}

void d4np_arena_reset(d4np_arena_t *arena)
{
    if (arena != NULL) {
        arena->offset = 0;
    }
}

void d4np_arena_destroy(d4np_arena_t *arena)
{
    if (arena == NULL || arena->base == NULL) {
        return;
    }
    d4np_free(arena->allocator, arena->base, arena->capacity);
    arena->allocator = NULL;
    arena->base = NULL;
    arena->capacity = 0;
    arena->offset = 0;
}

size_t d4np_arena_used(const d4np_arena_t *arena)
{
    return (arena != NULL) ? arena->offset : 0;
}

size_t d4np_arena_remaining(const d4np_arena_t *arena)
{
    return (arena != NULL) ? (arena->capacity - arena->offset) : 0;
}
