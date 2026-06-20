/*
 * d4np-c — default libc-backed allocator.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * malloc/realloc/free already return memory aligned for any scalar type (alignof(max_align_t)).
 * Over-aligned requests beyond that are not supported by this default allocator and yield NULL;
 * callers needing larger alignment use the arena/pool allocators (mem module), which honor it.
 */
#include "d4np/core/d4np_allocator.h"

#include <stdalign.h>
#include <stdlib.h>

/* The widest fundamental alignment, portably: malloc is guaranteed to satisfy it. We avoid
 * max_align_t because some C compilers (notably MSVC) do not expose it in C mode. */
typedef union d4np_max_align {
    long double ld;
    long long ll;
    void *p;
    void (*fp)(void);
} d4np_max_align_t;

static int align_ok(size_t align)
{
    /* A zero alignment means "no special requirement". Anything up to the platform's max
     * scalar alignment is satisfied by malloc; reject larger to avoid silently misaligning. */
    return align <= alignof(d4np_max_align_t);
}

static void *default_alloc(void *ctx, size_t size, size_t align)
{
    (void)ctx;
    if (size == 0 || !align_ok(align)) {
        return NULL;
    }
    return malloc(size);
}

static void *default_realloc(void *ctx, void *ptr, size_t old_size, size_t new_size, size_t align)
{
    (void)ctx;
    (void)old_size;
    if (!align_ok(align)) {
        return NULL;
    }
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, new_size);
}

static void default_free(void *ctx, void *ptr, size_t size)
{
    (void)ctx;
    (void)size;
    free(ptr);
}

const d4np_allocator_t *d4np_allocator_default(void)
{
    static const d4np_allocator_t instance = {
        default_alloc,
        default_realloc,
        default_free,
        NULL,
    };
    return &instance;
}

void *d4np_alloc(const d4np_allocator_t *a, size_t size, size_t align)
{
    if (a == NULL) {
        a = d4np_allocator_default();
    }
    return a->alloc(a->ctx, size, align);
}

void *d4np_realloc(const d4np_allocator_t *a, void *ptr, size_t old_size, size_t new_size, size_t align)
{
    if (a == NULL) {
        a = d4np_allocator_default();
    }
    return a->realloc(a->ctx, ptr, old_size, new_size, align);
}

void d4np_free(const d4np_allocator_t *a, void *ptr, size_t size)
{
    if (a == NULL) {
        a = d4np_allocator_default();
    }
    a->free(a->ctx, ptr, size);
}
