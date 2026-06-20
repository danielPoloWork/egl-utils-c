/*
 * d4np-c — dynamic string builder (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/ds/string_builder.h"

#include <string.h>

enum { D4NP_SB_MIN_CAPACITY = 16 };

static d4np_status_t sb_ensure(d4np_string_builder_t *sb, size_t needed)
{
    /* `needed` counts the terminating NUL. */
    if (needed <= sb->capacity) {
        return D4NP_OK;
    }
    size_t newcap = (sb->capacity == 0) ? (size_t)D4NP_SB_MIN_CAPACITY : sb->capacity;
    while (newcap < needed) {
        if (newcap > (size_t)-1 / 2) {
            newcap = needed; /* avoid doubling past SIZE_MAX */
            break;
        }
        newcap *= 2;
    }
    void *p = d4np_realloc(sb->allocator, sb->data, sb->capacity, newcap, 1);
    if (p == NULL) {
        return D4NP_ERR_OUT_OF_MEMORY;
    }
    sb->data = (char *)p;
    sb->capacity = newcap;
    return D4NP_OK;
}

d4np_status_t d4np_sb_init(d4np_string_builder_t *sb, const d4np_allocator_t *allocator, size_t initial_capacity)
{
    if (sb == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    sb->allocator = (allocator != NULL) ? allocator : d4np_allocator_default();
    sb->data = NULL;
    sb->len = 0;
    sb->capacity = 0;
    if (initial_capacity > 0) {
        d4np_status_t rc = sb_ensure(sb, initial_capacity);
        if (rc != D4NP_OK) {
            return rc;
        }
        sb->data[0] = '\0';
    }
    return D4NP_OK;
}

void d4np_sb_destroy(d4np_string_builder_t *sb)
{
    if (sb == NULL || sb->data == NULL) {
        return;
    }
    d4np_free(sb->allocator, sb->data, sb->capacity);
    sb->data = NULL;
    sb->len = 0;
    sb->capacity = 0;
}

d4np_status_t d4np_sb_reserve(d4np_string_builder_t *sb, size_t min_capacity)
{
    if (sb == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    d4np_status_t rc = sb_ensure(sb, min_capacity);
    if (rc == D4NP_OK && sb->data != NULL && sb->len == 0) {
        sb->data[0] = '\0';
    }
    return rc;
}

d4np_status_t d4np_sb_append_view(d4np_string_builder_t *sb, d4np_str_view_t view)
{
    if (sb == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (view.len == 0) {
        d4np_status_t rc = sb_ensure(sb, sb->len + 1);
        if (rc != D4NP_OK) {
            return rc;
        }
        sb->data[sb->len] = '\0';
        return D4NP_OK;
    }
    if (view.len > (size_t)-1 - sb->len - 1) {
        return D4NP_ERR_OVERFLOW;
    }
    d4np_status_t rc = sb_ensure(sb, sb->len + view.len + 1);
    if (rc != D4NP_OK) {
        return rc;
    }
    memcpy(sb->data + sb->len, view.ptr, view.len);
    sb->len += view.len;
    sb->data[sb->len] = '\0';
    return D4NP_OK;
}

d4np_status_t d4np_sb_append(d4np_string_builder_t *sb, const char *cstr)
{
    return d4np_sb_append_view(sb, d4np_str_view_from_str(cstr));
}

d4np_status_t d4np_sb_append_char(d4np_string_builder_t *sb, char c)
{
    if (sb == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    d4np_status_t rc = sb_ensure(sb, sb->len + 2);
    if (rc != D4NP_OK) {
        return rc;
    }
    sb->data[sb->len] = c;
    sb->len++;
    sb->data[sb->len] = '\0';
    return D4NP_OK;
}

const char *d4np_sb_cstr(const d4np_string_builder_t *sb)
{
    if (sb == NULL || sb->data == NULL) {
        return "";
    }
    return sb->data;
}

d4np_str_view_t d4np_sb_view(const d4np_string_builder_t *sb)
{
    if (sb == NULL || sb->data == NULL) {
        return d4np_str_view_from_buf(NULL, 0);
    }
    return d4np_str_view_from_buf(sb->data, sb->len);
}

size_t d4np_sb_len(const d4np_string_builder_t *sb)
{
    return (sb != NULL) ? sb->len : 0;
}

void d4np_sb_clear(d4np_string_builder_t *sb)
{
    if (sb == NULL) {
        return;
    }
    sb->len = 0;
    if (sb->data != NULL) {
        sb->data[0] = '\0';
    }
}
