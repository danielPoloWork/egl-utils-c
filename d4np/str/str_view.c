/*
 * d4np-c — non-owning string views (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/str/str_view.h"

#include <string.h>

d4np_str_view_t d4np_str_view_from_str(const char *cstr)
{
    d4np_str_view_t v;
    v.ptr = cstr;
    v.len = (cstr != NULL) ? strlen(cstr) : 0;
    return v;
}

d4np_str_view_t d4np_str_view_from_buf(const char *buf, size_t len)
{
    d4np_str_view_t v;
    v.ptr = buf;
    v.len = (buf != NULL) ? len : 0;
    return v;
}

bool d4np_str_view_equals(d4np_str_view_t a, d4np_str_view_t b)
{
    if (a.len != b.len) {
        return false;
    }
    if (a.len == 0) {
        return true;
    }
    return memcmp(a.ptr, b.ptr, a.len) == 0;
}

bool d4np_str_view_split_next(d4np_str_view_t *sv, char sep, d4np_str_view_t *out)
{
    if (sv == NULL || out == NULL || sv->ptr == NULL) {
        return false;
    }

    const char *start = sv->ptr;
    for (size_t i = 0; i < sv->len; ++i) {
        if (start[i] == sep) {
            out->ptr = start;
            out->len = i;
            sv->ptr = start + i + 1;
            sv->len = sv->len - i - 1;
            return true;
        }
    }

    /* No separator left: emit the remainder as the final field, then mark exhausted so the
     * next call returns false. A NULL pointer is the exhausted sentinel. */
    out->ptr = start;
    out->len = sv->len;
    sv->ptr = NULL;
    sv->len = 0;
    return true;
}

size_t d4np_str_split(d4np_str_view_t sv, char sep, d4np_str_view_t *out, size_t max_out)
{
    size_t count = 0;
    d4np_str_view_t token;
    while (d4np_str_view_split_next(&sv, sep, &token)) {
        if (out != NULL && count < max_out) {
            out[count] = token;
        }
        ++count;
    }
    return count;
}
