/**
 * @file vector.h
 * @brief Generic dynamic array (vector).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A reusable, allocator-injected dynamic array over fixed-size elements (the element size is
 * set at init, so the same code serves any POD type). Growth is geometric (2x) to amortize
 * push to O(1). Elements are stored contiguously for cache-friendly iteration; the vector
 * copies element bytes in/out and never owns pointers the elements may contain (spec \#6).
 *
 * @ingroup d4np_ds
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

/**
 * @addtogroup d4np_ds
 * @{
 */

/** @brief A geometrically growing dynamic array over fixed-size elements. */
typedef struct d4np_vector {
    const d4np_allocator_t *allocator; /**< backing allocator the storage came from */
    unsigned char *data;               /**< contiguous element storage               */
    size_t elem_size;                  /**< bytes per element (fixed at init)         */
    size_t len;                        /**< number of elements stored                 */
    size_t capacity;                   /**< number of elements allocated              */
} d4np_vector_t;

/**
 * @brief Initialize a vector for elements of a fixed byte size.
 *
 * @param v                Vector to initialize.
 * @param allocator        Backing allocator, or NULL to select the default allocator.
 * @param elem_size        Bytes per element; must be greater than 0.
 * @param initial_capacity Initial capacity in elements; may be 0 for lazy first allocation.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INVALID_ARGUMENT, ::D4NP_ERR_OVERFLOW, or
 *         ::D4NP_ERR_OUT_OF_MEMORY on failure.
 */
d4np_status_t d4np_vector_init(d4np_vector_t *v, const d4np_allocator_t *allocator, size_t elem_size,
                               size_t initial_capacity);

/**
 * @brief Release the backing storage and zero the vector.
 * @param v Vector to destroy.
 * @note Safe on a zeroed vector.
 */
void d4np_vector_destroy(d4np_vector_t *v);

/**
 * @brief Ensure capacity for at least @p min_capacity elements.
 * @param v            Vector to grow.
 * @param min_capacity Minimum capacity in elements; no-op if already large enough.
 * @return ::D4NP_OK on success, or an error status on failure.
 */
d4np_status_t d4np_vector_reserve(d4np_vector_t *v, size_t min_capacity);

/**
 * @brief Append a copy of one element to the end of the vector.
 * @param v    Vector to append to.
 * @param elem Pointer to @c elem_size bytes to copy in.
 * @return ::D4NP_OK on success, or an error status on failure.
 * @note Amortized O(1).
 */
d4np_status_t d4np_vector_push(d4np_vector_t *v, const void *elem);

/**
 * @brief Remove the last element.
 * @param v   Vector to pop from.
 * @param out If non-NULL, the removed element is copied here first.
 * @return ::D4NP_OK on success; ::D4NP_ERR_NOT_FOUND when the vector is empty.
 */
d4np_status_t d4np_vector_pop(d4np_vector_t *v, void *out);

/**
 * @brief Pointer to the element at @p index.
 * @param v     Vector to index.
 * @param index Zero-based element index.
 * @return Pointer to the element, or NULL if out of range.
 * @note The returned pointer is invalidated by growth.
 */
void *d4np_vector_at(const d4np_vector_t *v, size_t index);

/** @brief Number of elements stored. */
size_t d4np_vector_len(const d4np_vector_t *v);
/** @brief Number of elements currently allocated. */
size_t d4np_vector_capacity(const d4np_vector_t *v);
/** @brief Whether the vector holds no elements. */
bool d4np_vector_is_empty(const d4np_vector_t *v);

/**
 * @brief Reset length to 0, keeping the allocated capacity.
 * @param v Vector to clear.
 */
void d4np_vector_clear(d4np_vector_t *v);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_DS_VECTOR_H */
