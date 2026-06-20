/*
 * d4np-c — lock-free single-producer / single-consumer ring buffer.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A bounded FIFO over fixed-size elements for one producer thread and one consumer thread
 * (SPSC). It is lock-free: the producer owns `tail`, the consumer owns `head`, and the two
 * synchronize through C11 atomics with acquire/release ordering — no mutex on the hot path
 * (spec #9). Capacity is rounded up to a power of two so index wrap is a mask. The head/tail
 * are monotonic counters, so all `capacity` slots are usable.
 *
 * Contract: exactly one producer calls push and exactly one consumer calls pop concurrently.
 * Using more than one producer or consumer is undefined (use the M4 concurrency primitives).
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

typedef struct d4np_ring_buffer {
    const d4np_allocator_t *allocator;
    unsigned char *buffer;
    size_t elem_size;
    size_t capacity; /* number of slots, a power of two */
    size_t mask;     /* capacity - 1                    */
    atomic_size_t head; /* consumer: index of the next element to read  */
    atomic_size_t tail; /* producer: index of the next slot to write    */
} d4np_ring_buffer_t;

/*
 * Initialize `rb` to hold elements of `elem_size` bytes (> 0) with at least `capacity` slots
 * (> 0; rounded up to a power of two), drawn from `allocator` (NULL -> default). Returns
 * D4NP_OK, D4NP_ERR_INVALID_ARGUMENT, D4NP_ERR_OVERFLOW, or D4NP_ERR_OUT_OF_MEMORY.
 */
d4np_status_t d4np_ring_buffer_init(d4np_ring_buffer_t *rb, const d4np_allocator_t *allocator, size_t elem_size,
                                    size_t capacity);

/* Release the backing storage and zero the buffer. Not concurrent with push/pop. */
void d4np_ring_buffer_destroy(d4np_ring_buffer_t *rb);

/* Producer side: copy one element in. Returns false if the buffer is full. Lock-free, O(1). */
bool d4np_ring_buffer_push(d4np_ring_buffer_t *rb, const void *elem);

/* Consumer side: copy one element out (if `out` non-NULL). Returns false if empty. O(1). */
bool d4np_ring_buffer_pop(d4np_ring_buffer_t *rb, void *out);

/* Approximate number of queued elements (exact only when quiescent). */
size_t d4np_ring_buffer_count(const d4np_ring_buffer_t *rb);

bool d4np_ring_buffer_is_empty(const d4np_ring_buffer_t *rb);
size_t d4np_ring_buffer_capacity(const d4np_ring_buffer_t *rb);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_DS_RING_BUFFER_H */
