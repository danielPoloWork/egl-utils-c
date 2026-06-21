/*
 * d4np-c — concurrency performance gate (M8.5).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Unlike bench_concurrency.c (which only *reports* numbers), this program *asserts* them: it
 * measures the hot paths of the uncontended primitives and exits non-zero if any exceeds a
 * ceiling. The ceilings are deliberately loose — roughly 20-50x the figures measured on a normal
 * developer machine (mutex ~15 ns, atomic_queue enq+deq ~57 ns) — so the gate flags only
 * catastrophic regressions (e.g. a syscall sneaking into a hot path), never CI-runner jitter.
 * Run in Release; a Debug build is far slower and would trip the gate. See ADR-0007.
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

enum { N = 500000 };

static int failures = 0;

static void check(const char *what, double ns_per_op, double ceiling_ns)
{
    const char *verdict = ns_per_op <= ceiling_ns ? "OK" : "FAIL";
    printf("  %-34s %8.2f ns/op  (ceiling %7.0f)  %s\n", what, ns_per_op, ceiling_ns, verdict);
    if (ns_per_op > ceiling_ns) {
        ++failures;
    }
}

static double bench_mutex(void)
{
    d4np_mutex_t m;
    if (d4np_mutex_init(&m) != D4NP_OK) {
        return 1e18;
    }
    double start = wall_seconds();
    for (long i = 0; i < N; ++i) {
        d4np_mutex_lock(&m);
        d4np_mutex_unlock(&m);
    }
    double end = wall_seconds();
    d4np_mutex_destroy(&m);
    return (end - start) * 1e9 / (double)N;
}

static double bench_semaphore(void)
{
    d4np_semaphore_t s;
    if (d4np_semaphore_init(&s, 0) != D4NP_OK) {
        return 1e18;
    }
    double start = wall_seconds();
    for (long i = 0; i < N; ++i) {
        d4np_semaphore_post(&s);
        (void)d4np_semaphore_trywait(&s);
    }
    double end = wall_seconds();
    d4np_semaphore_destroy(&s);
    return (end - start) * 1e9 / (double)N;
}

static double bench_atomic_queue(void)
{
    d4np_atomic_queue_t q;
    if (d4np_atomic_queue_init(&q, NULL, sizeof(int)) != D4NP_OK) {
        return 1e18;
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
    (void)sink;
    d4np_atomic_queue_destroy(&q);
    return (end - start) * 1e9 / (double)N;
}

int main(void)
{
    printf("d4np-c concurrency perf gate (v%s, %d ops each)\n", D4NP_VERSION_STRING, N);
    check("mutex lock+unlock", bench_mutex(), 1000.0);
    check("semaphore post+trywait", bench_semaphore(), 2000.0);
    check("atomic_queue enq+deq", bench_atomic_queue(), 2000.0);
    if (failures > 0) {
        fprintf(stderr, "perf gate: %d threshold(s) exceeded\n", failures);
        return 1;
    }
    printf("perf gate: all thresholds met\n");
    return 0;
}
