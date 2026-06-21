/*
 * d4np-c — micro-benchmarks for the memory allocators (#1-#5).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A deliberately small, dependency-free harness (clock()) that contrasts the O(1) arena/pool
 * fast paths against malloc/free. It reports nanoseconds-per-op; it is evidence of the shape
 * of the cost, not a tuned benchmark. See docs/benchmarks/ for methodology.
 */
#include "d4np_c.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum { ITERATIONS = 2000000, POOL_BLOCKS = 1024 };

static double ns_per_op(clock_t start, clock_t end, long ops)
{
    double seconds = (double)(end - start) / (double)CLOCKS_PER_SEC;
    return (seconds * 1e9) / (double)ops;
}

static void bench_arena(void)
{
    d4np_arena_t arena;
    if (d4np_arena_init(&arena, NULL, 64u * 1024u) != D4NP_OK) {
        fprintf(stderr, "arena init failed\n");
        return;
    }
    volatile unsigned long sink = 0;
    clock_t start = clock();
    for (long i = 0; i < ITERATIONS; ++i) {
        void *p = d4np_arena_alloc(&arena, 16, 16);
        if (p == NULL) {
            d4np_arena_reset(&arena);
            p = d4np_arena_alloc(&arena, 16, 16);
        }
        sink += (unsigned long)(size_t)p;
    }
    clock_t end = clock();
    d4np_arena_destroy(&arena);
    printf("arena_alloc(16): %.2f ns/op  (%ld ops)\n", ns_per_op(start, end, ITERATIONS), (long)ITERATIONS);
    (void)sink;
}

static void bench_pool(void)
{
    d4np_pool_t pool;
    if (d4np_pool_init(&pool, NULL, 32, POOL_BLOCKS) != D4NP_OK) {
        fprintf(stderr, "pool init failed\n");
        return;
    }
    volatile unsigned long sink = 0;
    clock_t start = clock();
    for (long i = 0; i < ITERATIONS; ++i) {
        void *p = d4np_pool_alloc(&pool);
        sink += (unsigned long)(size_t)p;
        d4np_pool_free(&pool, p);
    }
    clock_t end = clock();
    d4np_pool_destroy(&pool);
    printf("pool_alloc+free(32): %.2f ns/op  (%ld ops)\n", ns_per_op(start, end, ITERATIONS), (long)ITERATIONS);
    (void)sink;
}

static void bench_malloc_baseline(void)
{
    volatile unsigned long sink = 0;
    clock_t start = clock();
    for (long i = 0; i < ITERATIONS; ++i) {
        void *p = malloc(32);
        sink += (unsigned long)(size_t)p;
        free(p);
    }
    clock_t end = clock();
    printf("malloc+free(32) baseline: %.2f ns/op  (%ld ops)\n", ns_per_op(start, end, ITERATIONS), (long)ITERATIONS);
    (void)sink;
}

int main(void)
{
    printf("d4np-c mem benchmarks (v%s)\n", D4NP_VERSION_STRING);
    bench_arena();
    bench_pool();
    bench_malloc_baseline();
    return 0;
}
