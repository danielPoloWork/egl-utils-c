/*
 * d4np-c — pluggable allocator contract.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Design pillar #1 (explicit memory management): every data structure in d4np-c takes a
 * d4np_allocator_t and never calls malloc/free directly. The allocator is a small vtable plus
 * an opaque context, so callers can supply an arena, a fixed-block pool, or the default
 * libc-backed allocator returned by d4np_allocator_default().
 */
#ifndef D4NP_CORE_ALLOCATOR_H
#define D4NP_CORE_ALLOCATOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Allocator vtable. `ctx` is passed back to every callback so an implementation can carry
 * its own state. `align` is the required alignment in bytes (a power of two); implementations
 * must return memory aligned to at least `align`. Sizes are passed back on realloc/free so
 * bump/pool allocators need not store per-allocation headers.
 */
typedef struct d4np_allocator {
    void *(*alloc)(void *ctx, size_t size, size_t align);
    void *(*realloc)(void *ctx, void *ptr, size_t old_size, size_t new_size, size_t align);
    void (*free)(void *ctx, void *ptr, size_t size);
    void *ctx;
} d4np_allocator_t;

/*
 * The process-wide default allocator, backed by the C standard library. The returned pointer
 * is to a static instance and must not be freed. Suitable as the fallback whenever a caller
 * passes a NULL allocator.
 */
const d4np_allocator_t *d4np_allocator_default(void);

/*
 * Thin wrappers that tolerate a NULL allocator (falling back to d4np_allocator_default()) and
 * a zero/NULL where the C standard library would, so call sites stay terse:
 *   - d4np_alloc(a, 0, _) returns NULL.
 *   - d4np_free(a, NULL, _) is a no-op.
 */
void *d4np_alloc(const d4np_allocator_t *a, size_t size, size_t align);
void *d4np_realloc(const d4np_allocator_t *a, void *ptr, size_t old_size, size_t new_size, size_t align);
void d4np_free(const d4np_allocator_t *a, void *ptr, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CORE_ALLOCATOR_H */
