/*
 * d4np-c — counting semaphore.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A counting semaphore over the platform primitive: Win32 CreateSemaphore, and on POSIX a
 * mutex + condition variable (portable across Linux and macOS, where unnamed POSIX
 * semaphores are unavailable). The native state lives in opaque, aligned storage so this
 * header needs no platform includes (spec #14).
 *
 * Scope: this is an in-process counting semaphore for thread synchronization. The named,
 * cross-process (IPC) variant and its multi-process verification are tracked in Milestone 8.
 */
#ifndef D4NP_CONCURRENCY_SEMAPHORE_H
#define D4NP_CONCURRENCY_SEMAPHORE_H

#include <stdbool.h>

#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque storage sized for the largest native backing across Tier-1 platforms (the POSIX
 * mutex+cond+counter); a _Static_assert in the implementation verifies the fit. */
typedef struct d4np_semaphore {
    _Alignas(16) unsigned char opaque[192];
} d4np_semaphore_t;

/* Initialize with `initial_count` permits. Returns D4NP_OK or D4NP_ERR_INTERNAL. */
d4np_status_t d4np_semaphore_init(d4np_semaphore_t *s, unsigned int initial_count);

/* Destroy the semaphore. No thread may be waiting. */
void d4np_semaphore_destroy(d4np_semaphore_t *s);

/* Acquire one permit, blocking until one is available (P / down). */
void d4np_semaphore_wait(d4np_semaphore_t *s);

/* Try to acquire one permit without blocking; returns true on success. */
bool d4np_semaphore_trywait(d4np_semaphore_t *s);

/* Release one permit, waking a waiter if any (V / up). */
void d4np_semaphore_post(d4np_semaphore_t *s);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CONCURRENCY_SEMAPHORE_H */
