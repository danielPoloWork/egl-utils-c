/*
 * d4np-c — lock-free single-producer / single-consumer unbounded queue.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * An unbounded FIFO for exactly one producer thread and one consumer thread (SPSC), built as a
 * linked list of nodes with a dummy head (Vyukov-style). It is lock-free and never "full": each
 * enqueue allocates a node from the injected allocator, so it grows on demand and only fails on
 * out-of-memory (spec #13). Contrast with d4np_ring_buffer_t, which is bounded and array-backed.
 *
 * Synchronization rides on each node's atomic `next` link (release on publish, acquire on read);
 * `head` is owned by the consumer and `tail` by the producer, so neither needs to be atomic.
 *
 * Contract: exactly one producer calls enqueue and exactly one consumer calls dequeue.
 */
#ifndef D4NP_CONCURRENCY_ATOMIC_QUEUE_H
#define D4NP_CONCURRENCY_ATOMIC_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque node; its definition (with the atomic link) lives in the implementation. */
typedef struct d4np_atomic_queue_node d4np_atomic_queue_node;

typedef struct d4np_atomic_queue {
    const d4np_allocator_t *allocator;
    size_t elem_size;
    size_t node_size;                /* allocation size per node (header + element) */
    d4np_atomic_queue_node *head;    /* consumer-owned: points at the dummy/last-read node */
    d4np_atomic_queue_node *tail;    /* producer-owned: most recently enqueued node          */
} d4np_atomic_queue_t;

/*
 * Initialize `q` for elements of `elem_size` bytes (> 0), drawn from `allocator` (NULL ->
 * default). Allocates the initial dummy node. Returns D4NP_OK, D4NP_ERR_INVALID_ARGUMENT,
 * D4NP_ERR_OVERFLOW, or D4NP_ERR_OUT_OF_MEMORY.
 */
d4np_status_t d4np_atomic_queue_init(d4np_atomic_queue_t *q, const d4np_allocator_t *allocator, size_t elem_size);

/* Free every remaining node and zero the queue. Not concurrent with enqueue/dequeue. */
void d4np_atomic_queue_destroy(d4np_atomic_queue_t *q);

/* Producer side: append a copy of the element. Returns false only on allocation failure. */
bool d4np_atomic_queue_enqueue(d4np_atomic_queue_t *q, const void *elem);

/* Consumer side: remove the oldest element into `out` (if non-NULL). Returns false if empty. */
bool d4np_atomic_queue_dequeue(d4np_atomic_queue_t *q, void *out);

/* Consumer-side emptiness check (approximate while a producer runs). */
bool d4np_atomic_queue_is_empty(const d4np_atomic_queue_t *q);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CONCURRENCY_ATOMIC_QUEUE_H */
