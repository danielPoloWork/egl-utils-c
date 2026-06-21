/**
 * @file atomic_queue.h
 * @brief Lock-free single-producer / single-consumer unbounded queue.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * An unbounded FIFO for exactly one producer thread and one consumer thread (SPSC), built as a
 * linked list of nodes with a dummy head (Vyukov-style). It is lock-free and never "full": each
 * enqueue allocates a node from the injected allocator, so it grows on demand and only fails on
 * out-of-memory (spec \#13). Contrast with d4np_ring_buffer_t, which is bounded and array-backed.
 *
 * Synchronization rides on each node's atomic `next` link (release on publish, acquire on read);
 * `head` is owned by the consumer and `tail` by the producer, so neither needs to be atomic.
 *
 * Contract: exactly one producer calls enqueue and exactly one consumer calls dequeue.
 *
 * @ingroup d4np_concurrency
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

/**
 * @addtogroup d4np_concurrency
 * @{
 */

/** @brief Opaque node; its definition (with the atomic link) lives in the implementation. */
typedef struct d4np_atomic_queue_node d4np_atomic_queue_node;

/** @brief A lock-free single-producer / single-consumer unbounded FIFO queue. */
typedef struct d4np_atomic_queue {
    const d4np_allocator_t *allocator; /**< backing allocator the nodes come from */
    size_t elem_size;                  /**< element size in bytes */
    size_t node_size;                  /**< allocation size per node (header + element) */
    d4np_atomic_queue_node *head;      /**< consumer-owned: points at the dummy/last-read node */
    d4np_atomic_queue_node *tail;      /**< producer-owned: most recently enqueued node          */
} d4np_atomic_queue_t;

/**
 * @brief Initialize a queue, allocating the initial dummy node.
 * @param q         Queue to initialize.
 * @param allocator Backing allocator, or NULL to select the default allocator.
 * @param elem_size Element size in bytes; must be greater than 0.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INVALID_ARGUMENT, ::D4NP_ERR_OVERFLOW, or
 *         ::D4NP_ERR_OUT_OF_MEMORY on failure.
 */
d4np_status_t d4np_atomic_queue_init(d4np_atomic_queue_t *q, const d4np_allocator_t *allocator, size_t elem_size);

/**
 * @brief Free every remaining node and zero the queue.
 * @param q Queue to destroy.
 * @note Not concurrent with enqueue/dequeue.
 */
void d4np_atomic_queue_destroy(d4np_atomic_queue_t *q);

/**
 * @brief Producer side: append a copy of the element.
 * @param q    Queue to enqueue into.
 * @param elem Element to copy into the queue.
 * @return true on success; false only on allocation failure.
 * @note Must be called by exactly one producer thread; concurrent with dequeue from one consumer.
 */
bool d4np_atomic_queue_enqueue(d4np_atomic_queue_t *q, const void *elem);

/**
 * @brief Consumer side: remove the oldest element into @p out (if non-NULL).
 * @param q   Queue to dequeue from.
 * @param out Destination for the removed element, or NULL to discard it.
 * @return true on success; false if the queue is empty.
 * @note Must be called by exactly one consumer thread; concurrent with enqueue from one producer.
 */
bool d4np_atomic_queue_dequeue(d4np_atomic_queue_t *q, void *out);

/**
 * @brief Consumer-side emptiness check.
 * @param q Queue to inspect.
 * @return true if the queue appears empty.
 * @note Approximate while a producer runs.
 */
bool d4np_atomic_queue_is_empty(const d4np_atomic_queue_t *q);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CONCURRENCY_ATOMIC_QUEUE_H */
