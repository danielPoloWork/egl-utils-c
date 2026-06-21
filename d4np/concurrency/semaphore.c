/*
 * d4np-c — counting semaphore (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/concurrency/semaphore.h"

#include <assert.h>

#if defined(_WIN32)
#include <limits.h>
#include <windows.h>

#define D4NP_SEM_HANDLE(s) (*(HANDLE *)(s)->opaque)
_Static_assert(sizeof(HANDLE) <= sizeof(((d4np_semaphore_t *)0)->opaque),
               "d4np_semaphore_t opaque storage too small for HANDLE");

d4np_status_t d4np_semaphore_init(d4np_semaphore_t *s, unsigned int initial_count)
{
    if (s == NULL || initial_count > (unsigned int)LONG_MAX) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    HANDLE h = CreateSemaphore(NULL, (LONG)initial_count, LONG_MAX, NULL);
    if (h == NULL) {
        return D4NP_ERR_INTERNAL;
    }
    D4NP_SEM_HANDLE(s) = h;
    return D4NP_OK;
}

void d4np_semaphore_destroy(d4np_semaphore_t *s)
{
    if (s != NULL) {
        CloseHandle(D4NP_SEM_HANDLE(s));
    }
}

void d4np_semaphore_wait(d4np_semaphore_t *s)
{
    assert(s != NULL);
    WaitForSingleObject(D4NP_SEM_HANDLE(s), INFINITE);
}

bool d4np_semaphore_trywait(d4np_semaphore_t *s)
{
    if (s == NULL) {
        return false;
    }
    return WaitForSingleObject(D4NP_SEM_HANDLE(s), 0) == WAIT_OBJECT_0;
}

void d4np_semaphore_post(d4np_semaphore_t *s)
{
    assert(s != NULL);
    ReleaseSemaphore(D4NP_SEM_HANDLE(s), 1, NULL);
}

#else /* POSIX: mutex + condition variable (works on Linux and macOS) */
#include <pthread.h>

typedef struct d4np_sem_posix {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned long count;
} d4np_sem_posix;

#define D4NP_SEM(s) ((d4np_sem_posix *)(s)->opaque)
_Static_assert(sizeof(d4np_sem_posix) <= sizeof(((d4np_semaphore_t *)0)->opaque),
               "d4np_semaphore_t opaque storage too small for the POSIX backing");

d4np_status_t d4np_semaphore_init(d4np_semaphore_t *s, unsigned int initial_count)
{
    if (s == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    d4np_sem_posix *p = D4NP_SEM(s);
    if (pthread_mutex_init(&p->mutex, NULL) != 0) {
        return D4NP_ERR_INTERNAL;
    }
    if (pthread_cond_init(&p->cond, NULL) != 0) {
        (void)pthread_mutex_destroy(&p->mutex);
        return D4NP_ERR_INTERNAL;
    }
    p->count = initial_count;
    return D4NP_OK;
}

void d4np_semaphore_destroy(d4np_semaphore_t *s)
{
    if (s == NULL) {
        return;
    }
    d4np_sem_posix *p = D4NP_SEM(s);
    (void)pthread_cond_destroy(&p->cond);
    (void)pthread_mutex_destroy(&p->mutex);
}

void d4np_semaphore_wait(d4np_semaphore_t *s)
{
    assert(s != NULL);
    d4np_sem_posix *p = D4NP_SEM(s);
    (void)pthread_mutex_lock(&p->mutex);
    while (p->count == 0) {
        (void)pthread_cond_wait(&p->cond, &p->mutex);
    }
    p->count--;
    (void)pthread_mutex_unlock(&p->mutex);
}

bool d4np_semaphore_trywait(d4np_semaphore_t *s)
{
    if (s == NULL) {
        return false;
    }
    d4np_sem_posix *p = D4NP_SEM(s);
    (void)pthread_mutex_lock(&p->mutex);
    bool ok = (p->count > 0);
    if (ok) {
        p->count--;
    }
    (void)pthread_mutex_unlock(&p->mutex);
    return ok;
}

void d4np_semaphore_post(d4np_semaphore_t *s)
{
    assert(s != NULL);
    d4np_sem_posix *p = D4NP_SEM(s);
    (void)pthread_mutex_lock(&p->mutex);
    p->count++;
    (void)pthread_cond_signal(&p->cond);
    (void)pthread_mutex_unlock(&p->mutex);
}

#endif
