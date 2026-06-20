/*
 * d4np-c — file-system path joining (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/io/path.h"

#include <string.h>

static int path_is_sep(char c)
{
#if defined(_WIN32)
    return c == '/' || c == '\\';
#else
    return c == '/';
#endif
}

size_t d4np_path_combine(char *out, size_t out_size, const char *a, const char *b)
{
#if defined(_WIN32)
    const char sep = '\\';
#else
    const char sep = '/';
#endif
    if (a == NULL) {
        a = "";
    }
    if (b == NULL) {
        b = "";
    }

    size_t a_full = strlen(a);
    size_t alen = a_full;
    while (alen > 0 && path_is_sep(a[alen - 1])) {
        --alen; /* drop trailing separators on the left */
    }
    size_t blen = strlen(b);
    size_t bstart = 0;
    while (bstart < blen && path_is_sep(b[bstart])) {
        ++bstart; /* drop leading separators on the right */
    }
    size_t bcount = blen - bstart;
    int a_is_root = (a_full > 0 && alen == 0); /* `a` was nothing but separators */

    size_t total = 0;
    size_t w = 0;
#define D4NP_PUT(ch)                                                                                                   \
    do {                                                                                                               \
        if (out != NULL && (w + 1) < out_size) {                                                                       \
            out[w++] = (ch);                                                                                           \
        }                                                                                                              \
        ++total;                                                                                                       \
    } while (0)

    if (a_is_root) {
        D4NP_PUT(sep);
        for (size_t i = 0; i < bcount; ++i) {
            D4NP_PUT(b[bstart + i]);
        }
    } else {
        for (size_t i = 0; i < alen; ++i) {
            D4NP_PUT(a[i]);
        }
        if (alen > 0 && bcount > 0) {
            D4NP_PUT(sep);
        }
        for (size_t i = 0; i < bcount; ++i) {
            D4NP_PUT(b[bstart + i]);
        }
    }
#undef D4NP_PUT

    if (out != NULL && out_size > 0) {
        out[w] = '\0';
    }
    return total;
}
