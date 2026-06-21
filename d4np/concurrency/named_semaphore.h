/**
 * @file named_semaphore.h
 * @brief Named, cross-process (IPC) counting semaphore.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A counting semaphore identified by a name rather than by memory, so unrelated processes can
 * rendezvous on the same kernel object: POSIX `sem_open` (Linux, macOS) and the Windows named
 * `CreateSemaphore`. This complements the in-process ::d4np_semaphore_t (which lives in a single
 * address space) for the inter-process case (spec \#14; deferred from M4 to M8).
 *
 * Naming: pass a short, bare key (letters/digits/underscore, ASCII). The implementation maps it
 * to the platform's namespace (a leading '/' on POSIX). Keep it under ~28 characters — macOS caps
 * named-semaphore names at 31 including the prefix. The same key names the same semaphore in
 * every process.
 *
 * Lifetime: the kernel object is created by the first ::d4np_named_semaphore_open and persists
 * until it is removed. On POSIX call ::d4np_named_semaphore_unlink once (from any one process)
 * to remove the name; on Windows the object is reclaimed automatically when the last handle
 * closes (unlink is a no-op there). Each process pairs its own open with ::d4np_named_semaphore_close.
 *
 * @ingroup d4np_concurrency
 */
#ifndef D4NP_CONCURRENCY_NAMED_SEMAPHORE_H
#define D4NP_CONCURRENCY_NAMED_SEMAPHORE_H

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
 * @brief A handle to a named, cross-process counting semaphore.
 *
 * Opaque storage holds the platform handle (a `sem_t *` on POSIX, a `HANDLE` on Windows); a
 * _Static_assert in the implementation verifies the fit.
 */
typedef struct d4np_named_semaphore {
    _Alignas(16) unsigned char opaque[32]; /**< opaque platform storage */
} d4np_named_semaphore_t;

/**
 * @brief Create the named semaphore, or open it if it already exists.
 *
 * The first opener creates the object with @p initial_count permits; later openers in any process
 * attach to it and ignore @p initial_count.
 *
 * @param s             Handle to initialize.
 * @param name          Bare semaphore key shared across processes (see naming notes above).
 * @param initial_count Permits the object starts with when this call creates it.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INVALID_ARGUMENT for a NULL/empty argument or a count
 *         above the platform maximum; ::D4NP_ERR_INTERNAL on platform failure.
 */
d4np_status_t d4np_named_semaphore_open(d4np_named_semaphore_t *s, const char *name, unsigned int initial_count);

/**
 * @brief Detach this process from the semaphore, releasing its handle.
 * @param s Handle to close. The named object itself survives until unlinked (POSIX) or the last
 *          handle closes (Windows).
 */
void d4np_named_semaphore_close(d4np_named_semaphore_t *s);

/**
 * @brief Remove the named object from the system (POSIX); a no-op on Windows.
 * @param name The same key passed to ::d4np_named_semaphore_open.
 * @return ::D4NP_OK on success or when no such name exists; ::D4NP_ERR_INVALID_ARGUMENT for a
 *         NULL/empty name; ::D4NP_ERR_INTERNAL on other platform failure.
 */
d4np_status_t d4np_named_semaphore_unlink(const char *name);

/**
 * @brief Acquire one permit, blocking until one is available (P / down).
 * @param s Semaphore to wait on.
 * @note Safe to call from multiple threads and multiple processes.
 */
void d4np_named_semaphore_wait(d4np_named_semaphore_t *s);

/**
 * @brief Try to acquire one permit without blocking.
 * @param s Semaphore to wait on.
 * @return true if a permit was acquired; false otherwise.
 * @note Safe to call from multiple threads and multiple processes.
 */
bool d4np_named_semaphore_trywait(d4np_named_semaphore_t *s);

/**
 * @brief Release one permit, waking a waiter if any (V / up).
 * @param s Semaphore to post to.
 * @note Safe to call from multiple threads and multiple processes.
 */
void d4np_named_semaphore_post(d4np_named_semaphore_t *s);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CONCURRENCY_NAMED_SEMAPHORE_H */
