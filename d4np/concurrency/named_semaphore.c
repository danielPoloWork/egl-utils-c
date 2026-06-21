/*
 * d4np-c — named, cross-process (IPC) counting semaphore (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
/* sem_open/sem_wait/sem_post and mode_t are POSIX; the build compiles as strict -std=c11. */
#define _POSIX_C_SOURCE 200809L

#include "d4np/concurrency/named_semaphore.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <limits.h>
#include <windows.h>

#define D4NP_NSEM_HANDLE(s) (*(HANDLE *)(s)->opaque)
_Static_assert(sizeof(HANDLE) <= sizeof(((d4np_named_semaphore_t *)0)->opaque),
               "d4np_named_semaphore_t opaque storage too small for HANDLE");

d4np_status_t d4np_named_semaphore_open(d4np_named_semaphore_t *s, const char *name, unsigned int initial_count)
{
    if (s == NULL || name == NULL || name[0] == '\0' || initial_count > (unsigned int)LONG_MAX) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    /* CreateSemaphore creates the object or, if the name already exists, returns a handle to it
     * (GetLastError() == ERROR_ALREADY_EXISTS) — exactly the create-or-open contract. */
    HANDLE h = CreateSemaphoreA(NULL, (LONG)initial_count, LONG_MAX, name);
    if (h == NULL) {
        return D4NP_ERR_INTERNAL;
    }
    D4NP_NSEM_HANDLE(s) = h;
    return D4NP_OK;
}

void d4np_named_semaphore_close(d4np_named_semaphore_t *s)
{
    if (s != NULL) {
        CloseHandle(D4NP_NSEM_HANDLE(s));
    }
}

d4np_status_t d4np_named_semaphore_unlink(const char *name)
{
    /* Windows reclaims the object when its last handle closes; there is no separate unlink. */
    if (name == NULL || name[0] == '\0') {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    return D4NP_OK;
}

void d4np_named_semaphore_wait(d4np_named_semaphore_t *s)
{
    assert(s != NULL);
    WaitForSingleObject(D4NP_NSEM_HANDLE(s), INFINITE);
}

bool d4np_named_semaphore_trywait(d4np_named_semaphore_t *s)
{
    if (s == NULL) {
        return false;
    }
    return WaitForSingleObject(D4NP_NSEM_HANDLE(s), 0) == WAIT_OBJECT_0;
}

void d4np_named_semaphore_post(d4np_named_semaphore_t *s)
{
    assert(s != NULL);
    ReleaseSemaphore(D4NP_NSEM_HANDLE(s), 1, NULL);
}

#else /* POSIX: named semaphores via sem_open (Linux, macOS) */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <sys/stat.h>

typedef struct d4np_named_sem_posix {
    sem_t *sem;
} d4np_named_sem_posix;

#define D4NP_NSEM(s) ((d4np_named_sem_posix *)(s)->opaque)
_Static_assert(sizeof(d4np_named_sem_posix) <= sizeof(((d4np_named_semaphore_t *)0)->opaque),
               "d4np_named_semaphore_t opaque storage too small for the POSIX backing");

/* Map a bare key to a POSIX named-semaphore name: a single leading '/', no other slashes. The
 * 32-byte cap keeps within macOS's 31-char limit (PSEMNAMLEN). Returns false on overflow. */
static bool d4np_nsem_name(char *buf, size_t bufsz, const char *name)
{
    int n = snprintf(buf, bufsz, "/%s", name);
    return n > 0 && (size_t)n < bufsz;
}

d4np_status_t d4np_named_semaphore_open(d4np_named_semaphore_t *s, const char *name, unsigned int initial_count)
{
    if (s == NULL || name == NULL || name[0] == '\0' || initial_count > (unsigned int)SEM_VALUE_MAX) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    char path[32];
    if (!d4np_nsem_name(path, sizeof path, name)) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    /* O_CREAT without O_EXCL: create the object, or open it if the name already exists. */
    sem_t *sem = sem_open(path, O_CREAT, (mode_t)0600, initial_count);
    if (sem == SEM_FAILED) {
        return D4NP_ERR_INTERNAL;
    }
    D4NP_NSEM(s)->sem = sem;
    return D4NP_OK;
}

void d4np_named_semaphore_close(d4np_named_semaphore_t *s)
{
    if (s != NULL) {
        (void)sem_close(D4NP_NSEM(s)->sem);
    }
}

d4np_status_t d4np_named_semaphore_unlink(const char *name)
{
    if (name == NULL || name[0] == '\0') {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    char path[32];
    if (!d4np_nsem_name(path, sizeof path, name)) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (sem_unlink(path) != 0 && errno != ENOENT) {
        return D4NP_ERR_INTERNAL;
    }
    return D4NP_OK;
}

void d4np_named_semaphore_wait(d4np_named_semaphore_t *s)
{
    assert(s != NULL);
    /* Retry across signal interruptions so a stray EINTR never looks like a spurious return. */
    while (sem_wait(D4NP_NSEM(s)->sem) != 0 && errno == EINTR) {
    }
}

bool d4np_named_semaphore_trywait(d4np_named_semaphore_t *s)
{
    if (s == NULL) {
        return false;
    }
    int rc;
    while ((rc = sem_trywait(D4NP_NSEM(s)->sem)) != 0 && errno == EINTR) {
    }
    return rc == 0;
}

void d4np_named_semaphore_post(d4np_named_semaphore_t *s)
{
    assert(s != NULL);
    (void)sem_post(D4NP_NSEM(s)->sem);
}

#endif
