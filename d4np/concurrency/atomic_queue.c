/*
 * d4np-c — lock-free SPSC unbounded queue (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Dummy-node SPSC linked queue. The producer publishes a node by releasing it into
 * tail->next; the consumer acquires head->next to observe it, which establishes the
 * happens-before edge that makes the node's element bytes visible. The consumer frees the old
 * head only after that acquire, so the producer is provably done with it (no use-after-free).
 */
#include "d4np/concurrency/atomic_queue.h"

#include <stdatomic.h>
#include <string.h>

typedef union d4np_aq_max_align {
    long double ld;
    long long ll;
    void *p;
    void (*fp)(void);
} d4np_aq_max_align_t;

struct d4np_atomic_queue_node {
    _Atomic(struct d4np_atomic_queue_node *) next;
    /* element bytes follow at D4NP_AQ_DATA_OFFSET */
};

#define D4NP_AQ_ALIGN (_Alignof(d4np_aq_max_align_t))
#define D4NP_AQ_DATA_OFFSET                                                                                      \
    (((sizeof(struct d4np_atomic_queue_node) + (D4NP_AQ_ALIGN - 1)) / D4NP_AQ_ALIGN) * D4NP_AQ_ALIGN)

static unsigned char *node_data(d4np_atomic_queue_node *n)
{
    return (unsigned char *)n + D4NP_AQ_DATA_OFFSET;
}

static d4np_atomic_queue_node *node_alloc(d4np_atomic_queue_t *q)
{
    d4np_atomic_queue_node *n = (d4np_atomic_queue_node *)d4np_alloc(q->allocator, q->node_size, D4NP_AQ_ALIGN);
    if (n != NULL) {
        atomic_store_explicit(&n->next, NULL, memory_order_relaxed);
    }
    return n;
}

d4np_status_t d4np_atomic_queue_init(d4np_atomic_queue_t *q, const d4np_allocator_t *allocator, size_t elem_size)
{
    if (q == NULL || elem_size == 0) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (elem_size > ((size_t)-1) - D4NP_AQ_DATA_OFFSET) {
        return D4NP_ERR_OVERFLOW;
    }
    q->allocator = (allocator != NULL) ? allocator : d4np_allocator_default();
    q->elem_size = elem_size;
    q->node_size = D4NP_AQ_DATA_OFFSET + elem_size;

    d4np_atomic_queue_node *dummy = node_alloc(q);
    if (dummy == NULL) {
        return D4NP_ERR_OUT_OF_MEMORY;
    }
    q->head = dummy;
    q->tail = dummy;
    return D4NP_OK;
}

void d4np_atomic_queue_destroy(d4np_atomic_queue_t *q)
{
    if (q == NULL || q->head == NULL) {
        return;
    }
    d4np_atomic_queue_node *cur = q->head;
    while (cur != NULL) {
        d4np_atomic_queue_node *next = atomic_load_explicit(&cur->next, memory_order_relaxed);
        d4np_free(q->allocator, cur, q->node_size);
        cur = next;
    }
    q->head = NULL;
    q->tail = NULL;
}

bool d4np_atomic_queue_enqueue(d4np_atomic_queue_t *q, const void *elem)
{
    if (q == NULL || elem == NULL) {
        return false;
    }
    d4np_atomic_queue_node *n = node_alloc(q);
    if (n == NULL) {
        return false;
    }
    memcpy(node_data(n), elem, q->elem_size);
    /* Publish: the consumer that acquires this link will see the element bytes above. */
    atomic_store_explicit(&q->tail->next, n, memory_order_release);
    q->tail = n;
    return true;
}

bool d4np_atomic_queue_dequeue(d4np_atomic_queue_t *q, void *out)
{
    if (q == NULL) {
        return false;
    }
    d4np_atomic_queue_node *next = atomic_load_explicit(&q->head->next, memory_order_acquire);
    if (next == NULL) {
        return false; /* empty */
    }
    if (out != NULL) {
        memcpy(out, node_data(next), q->elem_size);
    }
    d4np_atomic_queue_node *old_head = q->head;
    q->head = next; /* `next` becomes the new dummy */
    d4np_free(q->allocator, old_head, q->node_size);
    return true;
}

bool d4np_atomic_queue_is_empty(const d4np_atomic_queue_t *q)
{
    if (q == NULL || q->head == NULL) {
        return true;
    }
    return atomic_load_explicit(&q->head->next, memory_order_acquire) == NULL;
}
