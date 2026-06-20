/*
 * d4np-c — lock-free SPSC ring buffer (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Memory ordering: the producer publishes an element by storing the new `tail` with
 * memory_order_release after writing the slot; the consumer observes it with an acquire load of
 * `tail`, which establishes a happens-before edge so the element bytes are visible. The mirror
 * holds for `head`. Each side reads its own index with relaxed order (it is the only writer).
 */
#include "d4np/ds/ring_buffer.h"

#include <stdalign.h>
#include <string.h>

typedef union d4np_ring_max_align {
    long double ld;
    long long ll;
    void *p;
    void (*fp)(void);
} d4np_ring_max_align_t;

static size_t next_pow2_at_least(size_t n)
{
    size_t cap = 1;
    while (cap < n) {
        if (cap > ((size_t)-1) / 2) {
            return cap;
        }
        cap *= 2;
    }
    return cap;
}

d4np_status_t d4np_ring_buffer_init(d4np_ring_buffer_t *rb, const d4np_allocator_t *allocator, size_t elem_size,
                                    size_t capacity)
{
    if (rb == NULL || elem_size == 0 || capacity == 0) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (allocator == NULL) {
        allocator = d4np_allocator_default();
    }
    size_t cap = next_pow2_at_least(capacity);
    if (cap > ((size_t)-1) / elem_size) {
        return D4NP_ERR_OVERFLOW;
    }
    void *buffer = d4np_alloc(allocator, cap * elem_size, alignof(d4np_ring_max_align_t));
    if (buffer == NULL) {
        return D4NP_ERR_OUT_OF_MEMORY;
    }

    rb->allocator = allocator;
    rb->buffer = (unsigned char *)buffer;
    rb->elem_size = elem_size;
    rb->capacity = cap;
    rb->mask = cap - 1;
    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
    return D4NP_OK;
}

void d4np_ring_buffer_destroy(d4np_ring_buffer_t *rb)
{
    if (rb == NULL || rb->buffer == NULL) {
        return;
    }
    d4np_free(rb->allocator, rb->buffer, rb->capacity * rb->elem_size);
    rb->buffer = NULL;
    rb->elem_size = 0;
    rb->capacity = 0;
    rb->mask = 0;
    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
}

bool d4np_ring_buffer_push(d4np_ring_buffer_t *rb, const void *elem)
{
    if (rb == NULL || elem == NULL) {
        return false;
    }
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    if (tail - head == rb->capacity) {
        return false; /* full */
    }
    memcpy(rb->buffer + ((tail & rb->mask) * rb->elem_size), elem, rb->elem_size);
    atomic_store_explicit(&rb->tail, tail + 1, memory_order_release);
    return true;
}

bool d4np_ring_buffer_pop(d4np_ring_buffer_t *rb, void *out)
{
    if (rb == NULL) {
        return false;
    }
    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    if (head == tail) {
        return false; /* empty */
    }
    if (out != NULL) {
        memcpy(out, rb->buffer + ((head & rb->mask) * rb->elem_size), rb->elem_size);
    }
    atomic_store_explicit(&rb->head, head + 1, memory_order_release);
    return true;
}

size_t d4np_ring_buffer_count(const d4np_ring_buffer_t *rb)
{
    if (rb == NULL) {
        return 0;
    }
    d4np_ring_buffer_t *m = (d4np_ring_buffer_t *)rb; /* atomic loads need a non-const object */
    size_t tail = atomic_load_explicit(&m->tail, memory_order_acquire);
    size_t head = atomic_load_explicit(&m->head, memory_order_acquire);
    return tail - head;
}

bool d4np_ring_buffer_is_empty(const d4np_ring_buffer_t *rb)
{
    return d4np_ring_buffer_count(rb) == 0;
}

size_t d4np_ring_buffer_capacity(const d4np_ring_buffer_t *rb)
{
    return (rb != NULL) ? rb->capacity : 0;
}
