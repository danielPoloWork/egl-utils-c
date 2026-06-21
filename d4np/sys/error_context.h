/**
 * @file error_context.h
 * @brief Thread-local error context.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A per-thread stack of (status, message) frames for tracing how an error propagated, without
 * threading an error object through every signature (spec \#22). Push a frame as you unwind a
 * failure ("could not open config", "startup failed"); a top-level handler pops the frames to
 * report the trail. State is entirely thread-local: one thread's context never affects another.
 *
 * @ingroup d4np_sys
 */
#ifndef D4NP_SYS_ERROR_CONTEXT_H
#define D4NP_SYS_ERROR_CONTEXT_H

#include <stdbool.h>
#include <stddef.h>

#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_sys
 * @{
 */

/** @brief Maximum frames kept per thread; deeper pushes are dropped (the earliest frames are kept). */
#define D4NP_ERROR_CONTEXT_MAX_DEPTH 16

/**
 * @brief Push a frame onto the calling thread's context and return @p status unchanged.
 *
 * Returning @p status unchanged lets the call compose with a return:
 * `return d4np_error_context_push(D4NP_ERR_IO, "open failed");`.
 *
 * @param status  Status to record on the frame and return.
 * @param message Message to record; copied (truncated to an internal bound). NULL is treated as empty.
 * @return @p status, unchanged.
 * @note Thread-local: one thread's context never affects another.
 */
d4np_status_t d4np_error_context_push(d4np_status_t status, const char *message);

/**
 * @brief Pop the most recent frame.
 *
 * On success writes the frame's status to @p out_status and a pointer to its NUL-terminated
 * message to @p out_message (both optional). The message pointer stays valid until the next push
 * at that depth, a clear, or thread exit.
 *
 * @param out_status  Optional; receives the frame's status.
 * @param out_message Optional; receives a pointer to the frame's NUL-terminated message.
 * @return true if a frame was popped; false when the context is empty.
 * @note Thread-local: one thread's context never affects another.
 */
bool d4np_error_context_pop(d4np_status_t *out_status, const char **out_message);

/**
 * @brief Discard all frames for the calling thread.
 * @note Thread-local: one thread's context never affects another.
 */
void d4np_error_context_clear(void);

/**
 * @brief Number of frames currently held by the calling thread.
 * @return The current frame count for the calling thread.
 * @note Thread-local: one thread's context never affects another.
 */
size_t d4np_error_context_depth(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_SYS_ERROR_CONTEXT_H */
