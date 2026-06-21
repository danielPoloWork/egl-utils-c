/**
 * @file semaphore.h
 * @brief Counting semaphore.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A counting semaphore over the platform primitive: Win32 CreateSemaphore, and on POSIX a
 * mutex + condition variable (portable across Linux and macOS, where unnamed POSIX
 * semaphores are unavailable). The native state lives in opaque, aligned storage so this
 * header needs no platform includes (spec \#14).
 *
 * Scope: this is an in-process counting semaphore for thread synchronization. For the named,
 * cross-process (IPC) variant see ::d4np_named_semaphore_t (named_semaphore.h).
 *
 * @ingroup d4np_concurrency
 */
#ifndef D4NP_CONCURRENCY_SEMAPHORE_H
#define D4NP_CONCURRENCY_SEMAPHORE_H

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
 * @brief An in-process counting semaphore over the native platform primitive.
 *
 * Opaque storage sized for the largest native backing across Tier-1 platforms (the POSIX
 * mutex+cond+counter); a _Static_assert in the implementation verifies the fit.
 */
typedef struct d4np_semaphore {
    _Alignas(16) unsigned char opaque[192]; /**< opaque platform storage */
} d4np_semaphore_t;

/**
 * @brief Initialize a semaphore.
 * @param s             Semaphore to initialize.
 * @param initial_count Number of permits the semaphore starts with.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INTERNAL on platform failure.
 */
d4np_status_t d4np_semaphore_init(d4np_semaphore_t *s, unsigned int initial_count);

/**
 * @brief Destroy the semaphore.
 * @param s Semaphore to destroy. No thread may be waiting.
 */
void d4np_semaphore_destroy(d4np_semaphore_t *s);

/**
 * @brief Acquire one permit, blocking until one is available (P / down).
 * @param s Semaphore to wait on.
 * @note Thread-safe.
 */
void d4np_semaphore_wait(d4np_semaphore_t *s);

/**
 * @brief Try to acquire one permit without blocking.
 * @param s Semaphore to wait on.
 * @return true on success; false otherwise.
 * @note Thread-safe.
 */
bool d4np_semaphore_trywait(d4np_semaphore_t *s);

/**
 * @brief Release one permit, waking a waiter if any (V / up).
 * @param s Semaphore to post to.
 * @note Thread-safe.
 */
void d4np_semaphore_post(d4np_semaphore_t *s);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CONCURRENCY_SEMAPHORE_H */
