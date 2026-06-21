/*
 * d4np-c — whole-file read/write helpers.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Convenience I/O for the common "slurp a file" and "save a file" cases, with explicit error
 * reporting and an allocator for the read buffer (spec #18, #19). Writes are durable and
 * atomic: the data is written to a sibling temp file, flushed to disk, then renamed over the
 * target, so a reader never observes a half-written file.
 */
#ifndef D4NP_IO_FILE_H
#define D4NP_IO_FILE_H

#include <stddef.h>

#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Read the entire file at `path` into a freshly allocated buffer (from `allocator`, or the
 * default when NULL). On success *out_data points to `*out_size` bytes the caller owns and must
 * release with d4np_free(allocator, *out_data, *out_size); an empty file yields {NULL, 0}.
 * Reads via growing chunks, so non-seekable inputs work too. Returns D4NP_OK,
 * D4NP_ERR_INVALID_ARGUMENT, D4NP_ERR_OUT_OF_MEMORY, or D4NP_ERR_IO.
 */
d4np_status_t d4np_file_read_all(const char *path, const d4np_allocator_t *allocator, unsigned char **out_data,
                                 size_t *out_size);

/*
 * Atomically write `size` bytes of `data` to `path` (creating or replacing it): write a temp
 * file alongside the target, flush it to stable storage, then rename it into place. Returns
 * D4NP_OK, D4NP_ERR_INVALID_ARGUMENT (NULL path, or NULL data with size > 0, or path too long),
 * or D4NP_ERR_IO.
 */
d4np_status_t d4np_file_write_all(const char *path, const void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_IO_FILE_H */
