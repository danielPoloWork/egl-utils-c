/**
 * @file file.h
 * @brief Whole-file read/write helpers.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Convenience I/O for the common "slurp a file" and "save a file" cases, with explicit error
 * reporting and an allocator for the read buffer (spec \#18, \#19). Writes are durable and
 * atomic: the data is written to a sibling temp file, flushed to disk, then renamed over the
 * target, so a reader never observes a half-written file.
 *
 * @ingroup d4np_io
 */
#ifndef D4NP_IO_FILE_H
#define D4NP_IO_FILE_H

#include <stddef.h>

#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_io
 * @{
 */

/**
 * @brief Read an entire file into a freshly allocated buffer.
 *
 * On success @p out_data points to @p out_size bytes the caller owns and must release with
 * d4np_free(allocator, *out_data, *out_size); an empty file yields {NULL, 0}. Reads via growing
 * chunks, so non-seekable inputs work too.
 *
 * @param path      Path of the file to read.
 * @param allocator Allocator for the read buffer, or NULL to select the default allocator.
 * @param out_data  Receives a pointer to the allocated bytes.
 * @param out_size  Receives the number of bytes read.
 * @return ::D4NP_OK, ::D4NP_ERR_INVALID_ARGUMENT, ::D4NP_ERR_OUT_OF_MEMORY, or ::D4NP_ERR_IO.
 */
d4np_status_t d4np_file_read_all(const char *path, const d4np_allocator_t *allocator, unsigned char **out_data,
                                 size_t *out_size);

/**
 * @brief Atomically write bytes to a file, creating or replacing it.
 *
 * Writes a temp file alongside the target, flushes it to stable storage, then renames it into
 * place.
 *
 * @param path Path of the file to write.
 * @param data Bytes to write.
 * @param size Number of bytes to write.
 * @return ::D4NP_OK, ::D4NP_ERR_INVALID_ARGUMENT (NULL path, or NULL data with size > 0, or
 *         path too long), or ::D4NP_ERR_IO.
 */
d4np_status_t d4np_file_write_all(const char *path, const void *data, size_t size);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_IO_FILE_H */
