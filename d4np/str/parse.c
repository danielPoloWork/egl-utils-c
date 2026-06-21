/*
 * d4np-c — robust string-to-number parsing (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/str/parse.h"

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static int digit_value(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    return -1;
}

d4np_status_t d4np_str_parse_int(d4np_str_view_t sv, int base, int64_t *out)
{
    if (out == NULL || sv.ptr == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (base != 0 && (base < 2 || base > 36)) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }

    const char *p = sv.ptr;
    size_t i = 0;
    size_t n = sv.len;

    while (i < n && isspace((unsigned char)p[i])) {
        ++i;
    }
    if (i >= n) {
        return D4NP_ERR_INVALID_ARGUMENT; /* empty / whitespace only */
    }

    bool negative = false;
    if (p[i] == '+' || p[i] == '-') {
        negative = (p[i] == '-');
        ++i;
    }

    if (base == 0) {
        if (i < n && p[i] == '0') {
            if (i + 1 < n && (p[i + 1] == 'x' || p[i + 1] == 'X')) {
                base = 16;
                i += 2;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    } else if (base == 16 && i + 1 < n && p[i] == '0' && (p[i + 1] == 'x' || p[i + 1] == 'X')) {
        i += 2;
    }

    /* Accumulate as unsigned; the signed limit depends on the sign. */
    const uint64_t limit = negative ? (uint64_t)INT64_MAX + 1u : (uint64_t)INT64_MAX;
    const uint64_t ubase = (uint64_t)base;
    uint64_t acc = 0;
    bool any_digit = false;

    for (; i < n; ++i) {
        int d = digit_value(p[i]);
        if (d < 0 || d >= base) {
            break;
        }
        any_digit = true;
        if (acc > (limit - (uint64_t)d) / ubase) {
            return D4NP_ERR_OVERFLOW;
        }
        acc = acc * ubase + (uint64_t)d;
    }

    if (!any_digit) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    while (i < n && isspace((unsigned char)p[i])) {
        ++i;
    }
    if (i != n) {
        return D4NP_ERR_INVALID_ARGUMENT; /* trailing garbage */
    }

    if (negative) {
        *out = (acc == (uint64_t)INT64_MAX + 1u) ? INT64_MIN : -(int64_t)acc;
    } else {
        *out = (int64_t)acc;
    }
    return D4NP_OK;
}

d4np_status_t d4np_str_parse_float(d4np_str_view_t sv, double *out)
{
    if (out == NULL || sv.ptr == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }

    size_t i = 0;
    size_t n = sv.len;
    while (i < n && isspace((unsigned char)sv.ptr[i])) {
        ++i;
    }
    while (n > i && isspace((unsigned char)sv.ptr[n - 1])) {
        --n;
    }
    size_t len = n - i;
    if (len == 0) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }

    /* strtod needs a NUL-terminated string; copy the trimmed token into a scratch buffer. */
    char buf[128];
    if (len >= sizeof(buf)) {
        return D4NP_ERR_INVALID_ARGUMENT; /* implausibly long float literal */
    }
    memcpy(buf, sv.ptr + i, len);
    buf[len] = '\0';

    errno = 0;
    char *end = NULL;
    double value = strtod(buf, &end);
    if (end != buf + len) {
        return D4NP_ERR_INVALID_ARGUMENT; /* not fully consumed */
    }
    if (errno == ERANGE && (value >= HUGE_VAL || value <= -HUGE_VAL)) {
        return D4NP_ERR_OVERFLOW;
    }
    *out = value;
    return D4NP_OK;
}
