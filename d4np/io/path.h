/*
 * d4np-c — file-system path joining.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Joins two path segments with the platform separator ('\\' on Windows, '/' elsewhere),
 * collapsing a trailing separator on the left and a leading one on the right so no duplicate
 * separator appears (spec #20). This is a syntactic join, not a full normalizer: it does not
 * resolve "." / ".." or check the file system.
 */
#ifndef D4NP_IO_PATH_H
#define D4NP_IO_PATH_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Write the join of `a` and `b` into `out` (NUL-terminated, truncated to `out_size`). NULL
 * segments are treated as empty. Returns the full length the result would have, excluding the
 * NUL — like snprintf, a return value >= out_size means the output was truncated, so callers
 * can size a buffer by calling once with out_size == 0.
 */
size_t d4np_path_combine(char *out, size_t out_size, const char *a, const char *b);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_IO_PATH_H */
