/*
 * d4np-c — non-owning string views.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A d4np_str_view_t is a (pointer, length) pair over memory the view does not own. It enables
 * zero-allocation tokenization and substring work: nothing here allocates or copies, and a
 * view never owns or frees the bytes it points at. Views are NOT guaranteed NUL-terminated —
 * print with `printf("%.*s", (int)v.len, v.ptr)`.
 */
#ifndef D4NP_STR_STR_VIEW_H
#define D4NP_STR_STR_VIEW_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct d4np_str_view {
    const char *ptr; /* start of the viewed bytes; NULL only for the empty/invalid view */
    size_t len;      /* number of bytes viewed                                          */
} d4np_str_view_t;

/* View over a NUL-terminated C string. from_str(NULL) yields {NULL, 0}. */
d4np_str_view_t d4np_str_view_from_str(const char *cstr);

/* View over an explicit buffer + length. A NULL buffer yields {NULL, 0}. */
d4np_str_view_t d4np_str_view_from_buf(const char *buf, size_t len);

/* Byte-exact equality. Two empty views are equal regardless of pointer. */
bool d4np_str_view_equals(d4np_str_view_t a, d4np_str_view_t b);

/*
 * Zero-allocation forward tokenizer. Splits the view in *sv on the single byte `sep`,
 * writing the next field into *out and advancing *sv past it (and past the separator).
 *
 * Returns true while a field was produced, false once the view is exhausted. Fields between
 * adjacent separators are empty views (len 0), so "a;;b" yields "a", "", "b", and a trailing
 * separator yields a final empty field ("a;" -> "a", ""). The empty view yields exactly one
 * empty field. Typical use:
 *
 *     d4np_str_view_t sv = d4np_str_view_from_str("Daniel;Polo;Architect");
 *     d4np_str_view_t token;
 *     while (d4np_str_view_split_next(&sv, ';', &token)) {
 *         printf("Token: %.*s\n", (int)token.len, token.ptr);
 *     }
 */
bool d4np_str_view_split_next(d4np_str_view_t *sv, char sep, d4np_str_view_t *out);

/*
 * Zero-allocation split of `sv` on `sep` into a caller-provided array. Writes at most
 * `max_out` fields into `out` and returns the TOTAL number of fields in `sv` (same field
 * semantics as d4np_str_view_split_next). If the return value exceeds `max_out` the output was
 * truncated; call with `max_out == 0` (and any `out`) to count first, then size the array.
 */
size_t d4np_str_split(d4np_str_view_t sv, char sep, d4np_str_view_t *out, size_t max_out);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_STR_STR_VIEW_H */
