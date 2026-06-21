/*
 * d4np-c — micro-benchmarks for the data-structure module (#6-#9).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Dependency-free (clock()) throughput probes for the containers: amortized vector push,
 * hash map put/get, and single-threaded ring-buffer push+pop. Reports nanoseconds-per-op as
 * evidence of the cost shape, not a tuned benchmark. See docs/benchmarks/ for methodology.
 */
#include "d4np_c.h"

#include <stdio.h>
#include <time.h>

enum { N = 1000000, RB_CAPACITY = 4096 };

static double ns_per_op(clock_t start, clock_t end, long ops)
{
    double seconds = (double)(end - start) / (double)CLOCKS_PER_SEC;
    return (seconds * 1e9) / (double)ops;
}

static void bench_vector_push(void)
{
    d4np_vector_t v;
    if (d4np_vector_init(&v, NULL, sizeof(int), 0) != D4NP_OK) {
        return;
    }
    clock_t start = clock();
    for (int i = 0; i < N; ++i) {
        (void)d4np_vector_push(&v, &i);
    }
    clock_t end = clock();
    printf("vector_push(int):         %.2f ns/op  (%d ops, final cap %zu)\n", ns_per_op(start, end, N), N,
           d4np_vector_capacity(&v));
    d4np_vector_destroy(&v);
}

static void bench_hashmap(void)
{
    d4np_hashmap_t m;
    if (d4np_hashmap_init(&m, NULL, sizeof(int), sizeof(int), 0, NULL, NULL) != D4NP_OK) {
        return;
    }
    clock_t start = clock();
    for (int i = 0; i < N; ++i) {
        (void)d4np_hashmap_put(&m, &i, &i);
    }
    clock_t mid = clock();
    volatile long sink = 0;
    for (int i = 0; i < N; ++i) {
        int *p = d4np_hashmap_get(&m, &i);
        sink += (p != NULL) ? *p : 0;
    }
    clock_t end = clock();
    printf("hashmap_put(int->int):    %.2f ns/op  (%d ops)\n", ns_per_op(start, mid, N), N);
    printf("hashmap_get(int->int):    %.2f ns/op  (%d ops)\n", ns_per_op(mid, end, N), N);
    (void)sink;
    d4np_hashmap_destroy(&m);
}

static void bench_ring_buffer(void)
{
    d4np_ring_buffer_t rb;
    if (d4np_ring_buffer_init(&rb, NULL, sizeof(int), RB_CAPACITY) != D4NP_OK) {
        return;
    }
    volatile long sink = 0;
    clock_t start = clock();
    for (int i = 0; i < N; ++i) {
        (void)d4np_ring_buffer_push(&rb, &i);
        int out = 0;
        (void)d4np_ring_buffer_pop(&rb, &out);
        sink += out;
    }
    clock_t end = clock();
    printf("ring_buffer push+pop:     %.2f ns/op  (%d ops)\n", ns_per_op(start, end, N), N);
    (void)sink;
    d4np_ring_buffer_destroy(&rb);
}

int main(void)
{
    printf("d4np-c ds benchmarks (v%s)\n", D4NP_VERSION_STRING);
    bench_vector_push();
    bench_hashmap();
    bench_ring_buffer();
    return 0;
}
