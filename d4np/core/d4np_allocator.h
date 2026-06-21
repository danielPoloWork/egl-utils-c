/**
 * @file d4np_allocator.h
 * @brief Pluggable allocator contract.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Design pillar \#1 (explicit memory management): every data structure in d4np-c takes a
 * ::d4np_allocator_t and never calls malloc/free directly. The allocator is a small vtable plus
 * an opaque context, so callers can supply an arena, a fixed-block pool, or the default
 * libc-backed allocator returned by d4np_allocator_default().
 *
 * @ingroup d4np_core
 */
#ifndef D4NP_CORE_ALLOCATOR_H
#define D4NP_CORE_ALLOCATOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_core
 * @{
 */

/**
 * @brief Allocator vtable plus an opaque context.
 *
 * @c ctx is passed back to every callback so an implementation can carry its own state.
 * @c align is the required alignment in bytes (a power of two); implementations must return
 * memory aligned to at least @c align. Sizes are passed back on realloc/free so bump/pool
 * allocators need not store per-allocation headers.
 */
typedef struct d4np_allocator {
    void *(*alloc)(void *ctx, size_t size, size_t align); /**< allocate @c size bytes aligned to @c align */
    void *(*realloc)(void *ctx, void *ptr, size_t old_size, size_t new_size,
                     size_t align);                  /**< resize @c ptr from @c old_size to @c new_size */
    void (*free)(void *ctx, void *ptr, size_t size); /**< release @c ptr of @c size bytes */
    void *ctx;                                       /**< opaque state passed to every callback */
} d4np_allocator_t;

/**
 * @brief The process-wide default allocator, backed by the C standard library.
 *
 * @return Pointer to a static instance that must not be freed. Suitable as the fallback
 *         whenever a caller passes a NULL allocator.
 */
const d4np_allocator_t *d4np_allocator_default(void);

/**
 * @brief Allocate @p size bytes, tolerating a NULL allocator.
 *
 * @param a     Allocator to use, or NULL to fall back to d4np_allocator_default().
 * @param size  Number of bytes requested.
 * @param align Required alignment, a power of two.
 * @return Pointer to the allocation, or NULL when @p size is 0 or the allocation fails.
 */
void *d4np_alloc(const d4np_allocator_t *a, size_t size, size_t align);

/**
 * @brief Resize an allocation, tolerating a NULL allocator.
 *
 * @param a        Allocator to use, or NULL to fall back to d4np_allocator_default().
 * @param ptr      Existing allocation, or NULL to allocate afresh.
 * @param old_size Current size of @p ptr in bytes.
 * @param new_size Requested new size in bytes.
 * @param align    Required alignment, a power of two.
 * @return Pointer to the resized allocation, or NULL on failure.
 */
void *d4np_realloc(const d4np_allocator_t *a, void *ptr, size_t old_size, size_t new_size, size_t align);

/**
 * @brief Release an allocation, tolerating a NULL allocator.
 *
 * @param a    Allocator to use, or NULL to fall back to d4np_allocator_default().
 * @param ptr  Allocation to release; NULL is a no-op.
 * @param size Size of @p ptr in bytes, as passed at allocation time.
 */
void d4np_free(const d4np_allocator_t *a, void *ptr, size_t size);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CORE_ALLOCATOR_H */
