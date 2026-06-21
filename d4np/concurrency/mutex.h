/**
 * @file mutex.h
 * @brief Portable mutex.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A thin, non-recursive mutual-exclusion lock over the platform primitive (Win32
 * CRITICAL_SECTION / POSIX pthread_mutex_t). The native object is held in opaque, suitably
 * aligned storage so this public header pulls in neither <windows.h> nor <pthread.h>
 * (spec \#11). Not recursive: locking from the owning thread twice is undefined.
 *
 * @ingroup d4np_concurrency
 */
#ifndef D4NP_CONCURRENCY_MUTEX_H
#define D4NP_CONCURRENCY_MUTEX_H

#include <stdbool.h>

#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_concurrency
 * @{
 */

/**
 * @brief A non-recursive mutual-exclusion lock over the native platform primitive.
 *
 * Opaque storage sized to hold the largest native mutex across Tier-1 platforms; a
 * _Static_assert in the implementation verifies the fit.
 */
typedef struct d4np_mutex {
    _Alignas(16) unsigned char opaque[64]; /**< opaque platform storage */
} d4np_mutex_t;

/**
 * @brief Initialize a mutex.
 * @param m Mutex to initialize.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INTERNAL on platform failure.
 */
d4np_status_t d4np_mutex_init(d4np_mutex_t *m);

/**
 * @brief Destroy a mutex.
 * @param m Mutex to destroy. Must not be locked.
 * @note Safe to call once after a successful init.
 */
void d4np_mutex_destroy(d4np_mutex_t *m);

/**
 * @brief Acquire the lock, blocking until it is available.
 * @param m Mutex to lock.
 * @note Thread-safe. Not recursive: locking from the owning thread twice is undefined.
 */
void d4np_mutex_lock(d4np_mutex_t *m);

/**
 * @brief Release the lock.
 * @param m Mutex to unlock.
 * @note Thread-safe.
 */
void d4np_mutex_unlock(d4np_mutex_t *m);

/**
 * @brief Try to acquire the lock without blocking.
 * @param m Mutex to lock.
 * @return true if the lock was taken; false otherwise.
 * @note Thread-safe.
 */
bool d4np_mutex_trylock(d4np_mutex_t *m);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CONCURRENCY_MUTEX_H */
