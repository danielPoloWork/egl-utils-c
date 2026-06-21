/**
 * @file str_view.h
 * @brief Non-owning string views.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A d4np_str_view_t is a (pointer, length) pair over memory the view does not own. It enables
 * zero-allocation tokenization and substring work: nothing here allocates or copies, and a
 * view never owns or frees the bytes it points at. Views are NOT guaranteed NUL-terminated —
 * print with `printf("%.*s", (int)v.len, v.ptr)`.
 *
 * @ingroup d4np_str
 */
#ifndef D4NP_STR_STR_VIEW_H
#define D4NP_STR_STR_VIEW_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_str
 * @{
 */

/** @brief A (pointer, length) pair over memory the view does not own. */
typedef struct d4np_str_view {
    const char *ptr; /**< start of the viewed bytes; NULL only for the empty/invalid view */
    size_t len;      /**< number of bytes viewed                                          */
} d4np_str_view_t;

/**
 * @brief Create a view over a NUL-terminated C string.
 * @param cstr NUL-terminated string, or NULL.
 * @return A view of @p cstr; a NULL @p cstr yields {NULL, 0}.
 */
d4np_str_view_t d4np_str_view_from_str(const char *cstr);

/**
 * @brief Create a view over an explicit buffer and length.
 * @param buf Start of the buffer, or NULL.
 * @param len Number of bytes to view.
 * @return A view of @p len bytes at @p buf; a NULL @p buf yields {NULL, 0}.
 */
d4np_str_view_t d4np_str_view_from_buf(const char *buf, size_t len);

/**
 * @brief Byte-exact equality of two views.
 * @param a First view.
 * @param b Second view.
 * @return true if the views hold identical bytes; two empty views are equal regardless of
 *         pointer.
 */
bool d4np_str_view_equals(d4np_str_view_t a, d4np_str_view_t b);

/**
 * @brief Zero-allocation forward tokenizer.
 *
 * Splits the view in @p sv on the single byte @p sep, writing the next field into @p out and
 * advancing @p sv past it (and past the separator).
 *
 * Fields between adjacent separators are empty views (len 0), so "a;;b" yields "a", "", "b",
 * and a trailing separator yields a final empty field ("a;" -> "a", ""). The empty view yields
 * exactly one empty field. Typical use:
 *
 *     d4np_str_view_t sv = d4np_str_view_from_str("Daniel;Polo;Architect");
 *     d4np_str_view_t token;
 *     while (d4np_str_view_split_next(&sv, ';', &token)) {
 *         printf("Token: %.*s\n", (int)token.len, token.ptr);
 *     }
 *
 * @param sv  View to tokenize and advance.
 * @param sep Separator byte.
 * @param out Receives the next field.
 * @return true while a field was produced; false once the view is exhausted.
 */
bool d4np_str_view_split_next(d4np_str_view_t *sv, char sep, d4np_str_view_t *out);

/**
 * @brief Zero-allocation split of @p sv on @p sep into a caller-provided array.
 *
 * Writes at most @p max_out fields into @p out (same field semantics as
 * ::d4np_str_view_split_next).
 *
 * @param sv      View to split.
 * @param sep     Separator byte.
 * @param out     Array receiving up to @p max_out fields.
 * @param max_out Capacity of @p out in fields.
 * @return The TOTAL number of fields in @p sv. If the return value exceeds @p max_out the
 *         output was truncated; call with @p max_out == 0 (and any @p out) to count first, then
 *         size the array.
 */
size_t d4np_str_split(d4np_str_view_t sv, char sep, d4np_str_view_t *out, size_t max_out);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_STR_STR_VIEW_H */
