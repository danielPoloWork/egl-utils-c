/*
 * d4np-c — generic hash map (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/ds/hashmap.h"

#include <stdalign.h>
#include <string.h>

enum { D4NP_HASHMAP_MIN_CAPACITY = 8 };
enum { SLOT_EMPTY = 0, SLOT_OCCUPIED = 1, SLOT_DELETED = 2 };

typedef union d4np_hashmap_max_align {
    long double ld;
    long long ll;
    void *p;
    void (*fp)(void);
} d4np_hashmap_max_align_t;

/* ---- defaults ---------------------------------------------------------- */

static uint64_t default_hash(const void *key, size_t key_size)
{
    /* FNV-1a (64-bit). A public d4np_hash_fnv1a lands in the sys module (#25). */
    const unsigned char *p = (const unsigned char *)key;
    uint64_t h = 1469598103934665603ULL; /* offset basis */
    for (size_t i = 0; i < key_size; ++i) {
        h ^= (uint64_t)p[i];
        h *= 1099511628211ULL; /* prime */
    }
    return h;
}

static bool default_key_eq(const void *a, const void *b, size_t key_size)
{
    return memcmp(a, b, key_size) == 0;
}

/* ---- helpers ----------------------------------------------------------- */

static size_t round_up(size_t value, size_t multiple)
{
    return (value + (multiple - 1)) & ~(multiple - 1);
}

static size_t next_pow2_at_least(size_t n)
{
    size_t cap = D4NP_HASHMAP_MIN_CAPACITY;
    while (cap < n) {
        if (cap > ((size_t)-1) / 2) {
            return cap; /* saturate; callers also guard against overflow */
        }
        cap *= 2;
    }
    return cap;
}

static unsigned char *key_slot(const d4np_hashmap_t *m, size_t i)
{
    return m->keys + (i * m->key_stride);
}

static unsigned char *value_slot(const d4np_hashmap_t *m, size_t i)
{
    return m->values + (i * m->value_stride);
}

/*
 * Locate the slot for `key`. Sets *found to whether the key is present. On a miss, returns the
 * slot where the key should be inserted (reusing the first tombstone seen if any).
 */
static size_t locate(const d4np_hashmap_t *m, const void *key, bool *found)
{
    *found = false;
    size_t mask = m->capacity - 1;
    size_t start = (size_t)m->hash(key, m->key_size) & mask;
    size_t first_tomb = (size_t)-1;

    for (size_t probe = 0; probe < m->capacity; ++probe) {
        size_t i = (start + probe) & mask;
        uint8_t st = m->control[i];
        if (st == SLOT_EMPTY) {
            return (first_tomb != (size_t)-1) ? first_tomb : i;
        }
        if (st == SLOT_OCCUPIED && m->key_eq(key_slot(m, i), key, m->key_size)) {
            *found = true;
            return i;
        }
        if (st == SLOT_DELETED && first_tomb == (size_t)-1) {
            first_tomb = i;
        }
    }
    return (first_tomb != (size_t)-1) ? first_tomb : 0;
}

static d4np_status_t resize(d4np_hashmap_t *m, size_t new_capacity)
{
    const size_t align = alignof(d4np_hashmap_max_align_t);

    if (new_capacity > ((size_t)-1) / m->key_stride || new_capacity > ((size_t)-1) / m->value_stride) {
        return D4NP_ERR_OVERFLOW;
    }
    uint8_t *nc = (uint8_t *)d4np_alloc(m->allocator, new_capacity, align);
    unsigned char *nk = (unsigned char *)d4np_alloc(m->allocator, new_capacity * m->key_stride, align);
    unsigned char *nv = (unsigned char *)d4np_alloc(m->allocator, new_capacity * m->value_stride, align);
    if (nc == NULL || nk == NULL || nv == NULL) {
        d4np_free(m->allocator, nc, new_capacity);
        d4np_free(m->allocator, nk, new_capacity * m->key_stride);
        d4np_free(m->allocator, nv, new_capacity * m->value_stride);
        return D4NP_ERR_OUT_OF_MEMORY;
    }
    memset(nc, SLOT_EMPTY, new_capacity);

    /* Swap in the new (empty) table, then re-insert the live entries from the old one. */
    uint8_t *oc = m->control;
    unsigned char *ok = m->keys;
    unsigned char *ov = m->values;
    size_t old_cap = m->capacity;

    m->control = nc;
    m->keys = nk;
    m->values = nv;
    m->capacity = new_capacity;
    m->tombstones = 0;
    /* m->len is unchanged: the same entries are re-inserted. */

    size_t mask = new_capacity - 1;
    for (size_t i = 0; i < old_cap; ++i) {
        if (oc[i] != SLOT_OCCUPIED) {
            continue;
        }
        const unsigned char *k = ok + (i * m->key_stride);
        size_t start = (size_t)m->hash(k, m->key_size) & mask;
        for (size_t probe = 0; probe < new_capacity; ++probe) {
            size_t j = (start + probe) & mask;
            if (m->control[j] == SLOT_EMPTY) {
                m->control[j] = (uint8_t)SLOT_OCCUPIED;
                memcpy(key_slot(m, j), k, m->key_size);
                memcpy(value_slot(m, j), ov + (i * m->value_stride), m->value_size);
                break;
            }
        }
    }

    d4np_free(m->allocator, oc, old_cap);
    d4np_free(m->allocator, ok, old_cap * m->key_stride);
    d4np_free(m->allocator, ov, old_cap * m->value_stride);
    return D4NP_OK;
}

/* ---- public API -------------------------------------------------------- */

d4np_status_t d4np_hashmap_init(d4np_hashmap_t *m, const d4np_allocator_t *allocator, size_t key_size,
                                size_t value_size, size_t initial_capacity, d4np_hash_fn hash, d4np_key_eq_fn key_eq)
{
    if (m == NULL || key_size == 0 || value_size == 0) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    const size_t align = alignof(d4np_hashmap_max_align_t);

    m->allocator = (allocator != NULL) ? allocator : d4np_allocator_default();
    m->control = NULL;
    m->keys = NULL;
    m->values = NULL;
    m->key_size = key_size;
    m->value_size = value_size;
    m->key_stride = round_up(key_size, align);
    m->value_stride = round_up(value_size, align);
    m->capacity = 0;
    m->len = 0;
    m->tombstones = 0;
    m->hash = (hash != NULL) ? hash : default_hash;
    m->key_eq = (key_eq != NULL) ? key_eq : default_key_eq;

    if (initial_capacity > 0) {
        return resize(m, next_pow2_at_least(initial_capacity));
    }
    return D4NP_OK;
}

void d4np_hashmap_destroy(d4np_hashmap_t *m)
{
    if (m == NULL || m->control == NULL) {
        return;
    }
    d4np_free(m->allocator, m->control, m->capacity);
    d4np_free(m->allocator, m->keys, m->capacity * m->key_stride);
    d4np_free(m->allocator, m->values, m->capacity * m->value_stride);
    m->control = NULL;
    m->keys = NULL;
    m->values = NULL;
    m->capacity = 0;
    m->len = 0;
    m->tombstones = 0;
}

d4np_status_t d4np_hashmap_put(d4np_hashmap_t *m, const void *key, const void *value)
{
    if (m == NULL || key == NULL || value == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    /* Grow at a 0.75 load factor (counting tombstones, which a resize reclaims). */
    if ((m->len + m->tombstones + 1) * 4 >= m->capacity * 3) {
        size_t grown = (m->capacity == 0) ? (size_t)D4NP_HASHMAP_MIN_CAPACITY : m->capacity * 2;
        d4np_status_t rc = resize(m, grown);
        if (rc != D4NP_OK) {
            return rc;
        }
    }

    bool found = false;
    size_t i = locate(m, key, &found);
    if (found) {
        memcpy(value_slot(m, i), value, m->value_size);
        return D4NP_OK;
    }
    if (m->control[i] == SLOT_DELETED) {
        m->tombstones--;
    }
    m->control[i] = (uint8_t)SLOT_OCCUPIED;
    memcpy(key_slot(m, i), key, m->key_size);
    memcpy(value_slot(m, i), value, m->value_size);
    m->len++;
    return D4NP_OK;
}

void *d4np_hashmap_get(const d4np_hashmap_t *m, const void *key)
{
    if (m == NULL || key == NULL || m->capacity == 0) {
        return NULL;
    }
    bool found = false;
    size_t i = locate(m, key, &found);
    return found ? value_slot(m, i) : NULL;
}

bool d4np_hashmap_contains(const d4np_hashmap_t *m, const void *key)
{
    return d4np_hashmap_get(m, key) != NULL;
}

bool d4np_hashmap_remove(d4np_hashmap_t *m, const void *key)
{
    if (m == NULL || key == NULL || m->capacity == 0) {
        return false;
    }
    bool found = false;
    size_t i = locate(m, key, &found);
    if (!found) {
        return false;
    }
    m->control[i] = (uint8_t)SLOT_DELETED;
    m->len--;
    m->tombstones++;
    return true;
}

size_t d4np_hashmap_len(const d4np_hashmap_t *m)
{
    return (m != NULL) ? m->len : 0;
}

void d4np_hashmap_clear(d4np_hashmap_t *m)
{
    if (m == NULL || m->control == NULL) {
        return;
    }
    memset(m->control, SLOT_EMPTY, m->capacity);
    m->len = 0;
    m->tombstones = 0;
}
