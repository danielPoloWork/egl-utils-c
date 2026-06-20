/*
 * d4np-c — minimal portable thread helper for tests (Win32 / pthread).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Test-only: spawns a thread running a void(*)(void*) with a packed argument. Used by the
 * multi-threaded ring-buffer and concurrency tests. Not part of the library's public surface.
 */
#ifndef D4NP_TEST_THREADS_H
#define D4NP_TEST_THREADS_H

#ifdef _WIN32
#include <windows.h>
typedef HANDLE test_thread_t;
#else
#include <pthread.h>
#include <sched.h>
typedef pthread_t test_thread_t;
#endif

typedef void (*test_thread_fn)(void *);

typedef struct test_thread_pack {
    test_thread_fn fn;
    void *arg;
} test_thread_pack;

#ifdef _WIN32
static DWORD WINAPI test_thread_trampoline_(LPVOID p)
{
    test_thread_pack *tp = (test_thread_pack *)p;
    tp->fn(tp->arg);
    return 0;
}
static inline test_thread_t test_thread_spawn(test_thread_pack *tp)
{
    return CreateThread(NULL, 0, test_thread_trampoline_, tp, 0, NULL);
}
static inline void test_thread_join(test_thread_t t)
{
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
}
static inline void test_thread_yield(void)
{
    SwitchToThread();
}
#else
static inline void *test_thread_trampoline_(void *p)
{
    test_thread_pack *tp = (test_thread_pack *)p;
    tp->fn(tp->arg);
    return NULL;
}
static inline test_thread_t test_thread_spawn(test_thread_pack *tp)
{
    test_thread_t t;
    pthread_create(&t, NULL, test_thread_trampoline_, tp);
    return t;
}
static inline void test_thread_join(test_thread_t t)
{
    pthread_join(t, NULL);
}
static inline void test_thread_yield(void)
{
    sched_yield();
}
#endif

#endif /* D4NP_TEST_THREADS_H */
