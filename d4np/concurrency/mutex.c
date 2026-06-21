/*
 * d4np-c — portable mutex (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/concurrency/mutex.h"

#include <assert.h>

#if defined(_WIN32)
#include <windows.h>
#define D4NP_MTX(m) ((CRITICAL_SECTION *)(m)->opaque)
_Static_assert(sizeof(CRITICAL_SECTION) <= sizeof(((d4np_mutex_t *)0)->opaque),
               "d4np_mutex_t opaque storage too small for CRITICAL_SECTION");
#else
#include <pthread.h>
#define D4NP_MTX(m) ((pthread_mutex_t *)(m)->opaque)
_Static_assert(sizeof(pthread_mutex_t) <= sizeof(((d4np_mutex_t *)0)->opaque),
               "d4np_mutex_t opaque storage too small for pthread_mutex_t");
#endif

d4np_status_t d4np_mutex_init(d4np_mutex_t *m)
{
    if (m == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
#if defined(_WIN32)
    InitializeCriticalSection(D4NP_MTX(m));
    return D4NP_OK;
#else
    return (pthread_mutex_init(D4NP_MTX(m), NULL) == 0) ? D4NP_OK : D4NP_ERR_INTERNAL;
#endif
}

void d4np_mutex_destroy(d4np_mutex_t *m)
{
    if (m == NULL) {
        return;
    }
#if defined(_WIN32)
    DeleteCriticalSection(D4NP_MTX(m));
#else
    (void)pthread_mutex_destroy(D4NP_MTX(m));
#endif
}

void d4np_mutex_lock(d4np_mutex_t *m)
{
    assert(m != NULL);
#if defined(_WIN32)
    EnterCriticalSection(D4NP_MTX(m));
#else
    (void)pthread_mutex_lock(D4NP_MTX(m));
#endif
}

void d4np_mutex_unlock(d4np_mutex_t *m)
{
    assert(m != NULL);
#if defined(_WIN32)
    LeaveCriticalSection(D4NP_MTX(m));
#else
    (void)pthread_mutex_unlock(D4NP_MTX(m));
#endif
}

bool d4np_mutex_trylock(d4np_mutex_t *m)
{
    if (m == NULL) {
        return false;
    }
#if defined(_WIN32)
    return TryEnterCriticalSection(D4NP_MTX(m)) != 0;
#else
    return pthread_mutex_trylock(D4NP_MTX(m)) == 0;
#endif
}
