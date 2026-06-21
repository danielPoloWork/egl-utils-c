/**
 * @file ring_buffer.h
 * @brief Lock-free single-producer / single-consumer ring buffer.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A bounded FIFO over fixed-size elements for one producer thread and one consumer thread
 * (SPSC). It is lock-free: the producer owns `tail`, the consumer owns `head`, and the two
 * synchronize through C11 atomics with acquire/release ordering — no mutex on the hot path
 * (spec \#9). Capacity is rounded up to a power of two so index wrap is a mask. The head/tail
 * are monotonic counters, so all `capacity` slots are usable.
 *
 * Contract: exactly one producer calls push and exactly one consumer calls pop concurrently.
 * Using more than one producer or consumer is undefined (use the M4 concurrency primitives).
 *
 * @ingroup d4np_ds
 */
#ifndef D4NP_DS_RING_BUFFER_H
#define D4NP_DS_RING_BUFFER_H

#include <stdatomic.h>
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

/** @brief A bounded SPSC FIFO over fixed-size elements synchronized via C11 atomics. */
typedef struct d4np_ring_buffer {
    const d4np_allocator_t *allocator; /**< backing allocator the storage came from   */
    unsigned char *buffer;             /**< contiguous slot storage                    */
    size_t elem_size;                  /**< bytes per element (fixed at init)          */
    size_t capacity;                   /**< number of slots, a power of two */
    size_t mask;                       /**< capacity - 1                    */
    atomic_size_t head;                /**< consumer: index of the next element to read  */
    atomic_size_t tail;                /**< producer: index of the next slot to write    */
} d4np_ring_buffer_t;

/**
 * @brief Initialize a ring buffer for elements of a fixed byte size.
 *
 * @param rb        Buffer to initialize.
 * @param allocator Backing allocator, or NULL to select the default allocator.
 * @param elem_size Bytes per element; must be greater than 0.
 * @param capacity  Minimum number of slots; must be greater than 0 and is rounded up to a power
 *                  of two.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INVALID_ARGUMENT, ::D4NP_ERR_OVERFLOW, or
 *         ::D4NP_ERR_OUT_OF_MEMORY on failure.
 */
d4np_status_t d4np_ring_buffer_init(d4np_ring_buffer_t *rb, const d4np_allocator_t *allocator, size_t elem_size,
                                    size_t capacity);

/**
 * @brief Release the backing storage and zero the buffer.
 * @param rb Buffer to destroy.
 * @note Not concurrent with push/pop.
 */
void d4np_ring_buffer_destroy(d4np_ring_buffer_t *rb);

/**
 * @brief Producer side: copy one element in.
 * @param rb   Buffer to push to.
 * @param elem Pointer to @c elem_size bytes to copy in.
 * @return true on success, false if the buffer is full.
 * @note Lock-free, O(1). Must be called by exactly one producer thread.
 */
bool d4np_ring_buffer_push(d4np_ring_buffer_t *rb, const void *elem);

/**
 * @brief Consumer side: copy one element out.
 * @param rb  Buffer to pop from.
 * @param out If non-NULL, the popped element is copied here.
 * @return true on success, false if the buffer is empty.
 * @note Lock-free, O(1). Must be called by exactly one consumer thread.
 */
bool d4np_ring_buffer_pop(d4np_ring_buffer_t *rb, void *out);

/**
 * @brief Approximate number of queued elements.
 * @param rb Buffer to query.
 * @return The queued element count; exact only when quiescent.
 */
size_t d4np_ring_buffer_count(const d4np_ring_buffer_t *rb);

/** @brief Whether the buffer holds no elements (approximate; exact only when quiescent). */
bool d4np_ring_buffer_is_empty(const d4np_ring_buffer_t *rb);
/** @brief Number of usable slots (a power of two). */
size_t d4np_ring_buffer_capacity(const d4np_ring_buffer_t *rb);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_DS_RING_BUFFER_H */
