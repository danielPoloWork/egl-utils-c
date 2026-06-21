/*
 * d4np-c — thread-local error context.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A per-thread stack of (status, message) frames for tracing how an error propagated, without
 * threading an error object through every signature (spec #22). Push a frame as you unwind a
 * failure ("could not open config", "startup failed"); a top-level handler pops the frames to
 * report the trail. State is entirely thread-local: one thread's context never affects another.
 */
#ifndef D4NP_SYS_ERROR_CONTEXT_H
#define D4NP_SYS_ERROR_CONTEXT_H

#include <stdbool.h>
#include <stddef.h>

#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum frames kept per thread; deeper pushes are dropped (the earliest frames are kept). */
#define D4NP_ERROR_CONTEXT_MAX_DEPTH 16

/*
 * Push a frame onto the calling thread's context and return `status` unchanged, so it composes
 * with a return: `return d4np_error_context_push(D4NP_ERR_IO, "open failed");`. `message` is
 * copied (truncated to an internal bound); NULL is treated as empty.
 */
d4np_status_t d4np_error_context_push(d4np_status_t status, const char *message);

/*
 * Pop the most recent frame. On success writes its status to *out_status and a pointer to its
 * (NUL-terminated) message to *out_message (both optional) and returns true; returns false when
 * the context is empty. The message pointer stays valid until the next push at that depth, a
 * clear, or thread exit.
 */
bool d4np_error_context_pop(d4np_status_t *out_status, const char **out_message);

/* Discard all frames for the calling thread. */
void d4np_error_context_clear(void);

/* Number of frames currently held by the calling thread. */
size_t d4np_error_context_depth(void);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_SYS_ERROR_CONTEXT_H */
