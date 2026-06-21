/*
 * d4np-c — generic dynamic array (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/ds/vector.h"

#include <stdalign.h>
#include <string.h>

enum { D4NP_VECTOR_MIN_CAPACITY = 8 };

typedef union d4np_vector_max_align {
    long double ld;
    long long ll;
    void *p;
    void (*fp)(void);
} d4np_vector_max_align_t;

d4np_status_t d4np_vector_init(d4np_vector_t *v, const d4np_allocator_t *allocator, size_t elem_size,
                               size_t initial_capacity)
{
    if (v == NULL || elem_size == 0) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    v->allocator = (allocator != NULL) ? allocator : d4np_allocator_default();
    v->data = NULL;
    v->elem_size = elem_size;
    v->len = 0;
    v->capacity = 0;
    if (initial_capacity > 0) {
        return d4np_vector_reserve(v, initial_capacity);
    }
    return D4NP_OK;
}

void d4np_vector_destroy(d4np_vector_t *v)
{
    if (v == NULL || v->data == NULL) {
        return;
    }
    d4np_free(v->allocator, v->data, v->capacity * v->elem_size);
    v->data = NULL;
    v->len = 0;
    v->capacity = 0;
}

d4np_status_t d4np_vector_reserve(d4np_vector_t *v, size_t min_capacity)
{
    if (v == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (min_capacity <= v->capacity) {
        return D4NP_OK;
    }
    if (min_capacity > (size_t)-1 / v->elem_size) {
        return D4NP_ERR_OVERFLOW;
    }

    size_t new_bytes = min_capacity * v->elem_size;
    size_t old_bytes = v->capacity * v->elem_size;
    void *p = d4np_realloc(v->allocator, v->data, old_bytes, new_bytes, alignof(d4np_vector_max_align_t));
    if (p == NULL) {
        return D4NP_ERR_OUT_OF_MEMORY;
    }
    v->data = (unsigned char *)p;
    v->capacity = min_capacity;
    return D4NP_OK;
}

d4np_status_t d4np_vector_push(d4np_vector_t *v, const void *elem)
{
    if (v == NULL || elem == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (v->len == v->capacity) {
        size_t grown = (v->capacity == 0) ? (size_t)D4NP_VECTOR_MIN_CAPACITY : v->capacity * 2;
        d4np_status_t rc = d4np_vector_reserve(v, grown);
        if (rc != D4NP_OK) {
            return rc;
        }
    }
    memcpy(v->data + (v->len * v->elem_size), elem, v->elem_size);
    v->len++;
    return D4NP_OK;
}

d4np_status_t d4np_vector_pop(d4np_vector_t *v, void *out)
{
    if (v == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (v->len == 0) {
        return D4NP_ERR_NOT_FOUND;
    }
    v->len--;
    if (out != NULL) {
        memcpy(out, v->data + (v->len * v->elem_size), v->elem_size);
    }
    return D4NP_OK;
}

void *d4np_vector_at(const d4np_vector_t *v, size_t index)
{
    if (v == NULL || index >= v->len) {
        return NULL;
    }
    return v->data + (index * v->elem_size);
}

size_t d4np_vector_len(const d4np_vector_t *v)
{
    return (v != NULL) ? v->len : 0;
}

size_t d4np_vector_capacity(const d4np_vector_t *v)
{
    return (v != NULL) ? v->capacity : 0;
}

bool d4np_vector_is_empty(const d4np_vector_t *v)
{
    return (v == NULL) || (v->len == 0);
}

void d4np_vector_clear(d4np_vector_t *v)
{
    if (v != NULL) {
        v->len = 0;
    }
}
