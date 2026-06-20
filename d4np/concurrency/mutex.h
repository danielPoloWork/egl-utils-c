/*
 * d4np-c — portable mutex.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A thin, non-recursive mutual-exclusion lock over the platform primitive (Win32
 * CRITICAL_SECTION / POSIX pthread_mutex_t). The native object is held in opaque, suitably
 * aligned storage so this public header pulls in neither <windows.h> nor <pthread.h>
 * (spec #11). Not recursive: locking from the owning thread twice is undefined.
 */
#ifndef D4NP_CONCURRENCY_MUTEX_H
#define D4NP_CONCURRENCY_MUTEX_H

#include <stdbool.h>

#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque storage sized to hold the largest native mutex across Tier-1 platforms; a
 * _Static_assert in the implementation verifies the fit. */
typedef struct d4np_mutex {
    _Alignas(16) unsigned char opaque[64];
} d4np_mutex_t;

/* Initialize a mutex. Returns D4NP_OK or D4NP_ERR_INTERNAL (platform failure). */
d4np_status_t d4np_mutex_init(d4np_mutex_t *m);

/* Destroy a mutex. Must not be locked. Safe to call once after a successful init. */
void d4np_mutex_destroy(d4np_mutex_t *m);

/* Acquire (blocking) / release the lock. */
void d4np_mutex_lock(d4np_mutex_t *m);
void d4np_mutex_unlock(d4np_mutex_t *m);

/* Try to acquire without blocking; returns true if the lock was taken. */
bool d4np_mutex_trylock(d4np_mutex_t *m);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CONCURRENCY_MUTEX_H */
