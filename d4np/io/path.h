/**
 * @file path.h
 * @brief File-system path joining.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Joins two path segments with the platform separator ('\\' on Windows, '/' elsewhere),
 * collapsing a trailing separator on the left and a leading one on the right so no duplicate
 * separator appears (spec \#20). This is a syntactic join, not a full normalizer: it does not
 * resolve "." / ".." or check the file system.
 *
 * @ingroup d4np_io
 */
#ifndef D4NP_IO_PATH_H
#define D4NP_IO_PATH_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_io
 * @{
 */

/**
 * @brief Join two path segments into a caller-provided buffer.
 *
 * Writes the join of @p a and @p b into @p out (NUL-terminated, truncated to @p out_size). NULL
 * segments are treated as empty.
 *
 * @param out      Output buffer for the NUL-terminated result.
 * @param out_size Size of @p out in bytes.
 * @param a        Left segment, or NULL.
 * @param b        Right segment, or NULL.
 * @return The full length the result would have, excluding the NUL. Like snprintf, a return
 *         value >= @p out_size means the output was truncated, so callers can size a buffer by
 *         calling once with @p out_size == 0.
 */
size_t d4np_path_combine(char *out, size_t out_size, const char *a, const char *b);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_IO_PATH_H */
