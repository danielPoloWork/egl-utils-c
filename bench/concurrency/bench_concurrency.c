/*
 * d4np-c — micro-benchmarks for the concurrency module (#11-#14).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Uses a small monotonic wall clock (so the multi-threaded thread-pool figure is meaningful,
 * unlike process-CPU clock()). Reports ns/op for the uncontended primitives and tasks/sec for
 * the pool's dispatch path. Evidence of cost shape, not a tuned benchmark. See docs/benchmarks/.
 */
/* clock_gettime / CLOCK_MONOTONIC need POSIX.1; the build compiles as strict -std=c11. */
#define _POSIX_C_SOURCE 200809L

#include "d4np_c.h"

#include <stdio.h>

#if defined(_WIN32)
#include <windows.h>
static double wall_seconds(void)
{
    LARGE_INTEGER freq, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart / (double)freq.QuadPart;
}
#else
#include <time.h>
static double wall_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
#endif

enum { N = 2000000, POOL_TASKS = 1000000, POOL_WORKERS = 4 };

static void bench_mutex(void)
{
    d4np_mutex_t m;
    if (d4np_mutex_init(&m) != D4NP_OK) {
        return;
    }
    double start = wall_seconds();
    for (long i = 0; i < N; ++i) {
        d4np_mutex_lock(&m);
        d4np_mutex_unlock(&m);
    }
    double end = wall_seconds();
    printf("mutex lock+unlock (uncontended): %.2f ns/op  (%d ops)\n", (end - start) * 1e9 / (double)N, N);
    d4np_mutex_destroy(&m);
}

static void bench_atomic_queue(void)
{
    d4np_atomic_queue_t q;
    if (d4np_atomic_queue_init(&q, NULL, sizeof(int)) != D4NP_OK) {
        return;
    }
    volatile long sink = 0;
    double start = wall_seconds();
    for (int i = 0; i < N; ++i) {
        (void)d4np_atomic_queue_enqueue(&q, &i);
        int out = 0;
        (void)d4np_atomic_queue_dequeue(&q, &out);
        sink += out;
    }
    double end = wall_seconds();
    printf("atomic_queue enq+deq (1 thread): %.2f ns/op  (%d ops)\n", (end - start) * 1e9 / (double)N, N);
    (void)sink;
    d4np_atomic_queue_destroy(&q);
}

static void noop_task(void *arg)
{
    (void)arg;
}

static void bench_thread_pool(void)
{
    d4np_thread_pool_t pool;
    if (d4np_thread_pool_init(&pool, NULL, POOL_WORKERS) != D4NP_OK) {
        return;
    }
    double start = wall_seconds();
    for (int i = 0; i < POOL_TASKS; ++i) {
        (void)d4np_thread_pool_submit(&pool, noop_task, NULL);
    }
    d4np_thread_pool_destroy(&pool); /* drains all tasks */
    double end = wall_seconds();
    double seconds = end - start;
    printf("thread_pool dispatch (%d workers): %.2f M tasks/sec  (%d tasks in %.3f s)\n", POOL_WORKERS,
           (double)POOL_TASKS / seconds / 1e6, POOL_TASKS, seconds);
}

int main(void)
{
    printf("d4np-c concurrency benchmarks (v%s)\n", D4NP_VERSION_STRING);
    bench_mutex();
    bench_atomic_queue();
    bench_thread_pool();
    return 0;
}
