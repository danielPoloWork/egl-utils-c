/*
 * d4np-c — generic dynamic array (vector).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A reusable, allocator-injected dynamic array over fixed-size elements (the element size is
 * set at init, so the same code serves any POD type). Growth is geometric (2x) to amortize
 * push to O(1). Elements are stored contiguously for cache-friendly iteration; the vector
 * copies element bytes in/out and never owns pointers the elements may contain (spec #6).
 */
#ifndef D4NP_DS_VECTOR_H
#define D4NP_DS_VECTOR_H

#include <stdbool.h>
#include <stddef.h>

#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct d4np_vector {
    const d4np_allocator_t *allocator;
    unsigned char *data;
    size_t elem_size; /* bytes per element (fixed at init) */
    size_t len;       /* number of elements stored          */
    size_t capacity;  /* number of elements allocated       */
} d4np_vector_t;

/*
 * Initialize `v` for elements of `elem_size` bytes (> 0), drawn from `allocator` (NULL ->
 * default). `initial_capacity` may be 0 (lazy first allocation). Returns D4NP_OK,
 * D4NP_ERR_INVALID_ARGUMENT, D4NP_ERR_OVERFLOW, or D4NP_ERR_OUT_OF_MEMORY.
 */
d4np_status_t d4np_vector_init(d4np_vector_t *v, const d4np_allocator_t *allocator, size_t elem_size,
                               size_t initial_capacity);

/* Release the backing storage and zero the vector. Safe on a zeroed vector. */
void d4np_vector_destroy(d4np_vector_t *v);

/* Ensure capacity for at least `min_capacity` elements (no-op if already large enough). */
d4np_status_t d4np_vector_reserve(d4np_vector_t *v, size_t min_capacity);

/* Append a copy of `elem_size` bytes from `elem`. Amortized O(1). */
d4np_status_t d4np_vector_push(d4np_vector_t *v, const void *elem);

/*
 * Remove the last element. If `out` is non-NULL, copy it there first. Returns
 * D4NP_ERR_NOT_FOUND when the vector is empty.
 */
d4np_status_t d4np_vector_pop(d4np_vector_t *v, void *out);

/* Pointer to element `index`, or NULL if out of range. Invalidated by growth. */
void *d4np_vector_at(const d4np_vector_t *v, size_t index);

size_t d4np_vector_len(const d4np_vector_t *v);
size_t d4np_vector_capacity(const d4np_vector_t *v);
bool d4np_vector_is_empty(const d4np_vector_t *v);

/* Reset length to 0, keeping the allocated capacity. */
void d4np_vector_clear(d4np_vector_t *v);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_DS_VECTOR_H */
