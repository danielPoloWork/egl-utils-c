/*
 * d4np-c — tests for the lock-free SPSC ring buffer (#9).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Includes a real multi-threaded producer/consumer run: one producer thread streams a long
 * monotonic sequence while the main thread consumes it, asserting FIFO order and exact count.
 * Run under ThreadSanitizer in CI (ctest --preset tsan) to prove the atomics are race-free.
 */
#include "unity.h"

#include "d4np_c.h"

/* ---- minimal portable thread shim (test-only) -------------------------- */
#ifdef _WIN32
#include <windows.h>
typedef HANDLE test_thread_t;
#else
#include <pthread.h>
typedef pthread_t test_thread_t;
#endif

typedef void (*test_thread_fn)(void *);
typedef struct {
    test_thread_fn fn;
    void *arg;
} test_thread_pack;

#ifdef _WIN32
static DWORD WINAPI test_thread_trampoline(LPVOID p)
{
    test_thread_pack *tp = (test_thread_pack *)p;
    tp->fn(tp->arg);
    return 0;
}
static test_thread_t test_thread_spawn(test_thread_pack *tp)
{
    return CreateThread(NULL, 0, test_thread_trampoline, tp, 0, NULL);
}
static void test_thread_join(test_thread_t t)
{
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
}
#else
static void *test_thread_trampoline(void *p)
{
    test_thread_pack *tp = (test_thread_pack *)p;
    tp->fn(tp->arg);
    return NULL;
}
static test_thread_t test_thread_spawn(test_thread_pack *tp)
{
    test_thread_t t;
    pthread_create(&t, NULL, test_thread_trampoline, tp);
    return t;
}
static void test_thread_join(test_thread_t t)
{
    pthread_join(t, NULL);
}
#endif

/* ---- single-threaded correctness --------------------------------------- */

static void test_init_rejects_bad_args(void)
{
    d4np_ring_buffer_t rb;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_ring_buffer_init(NULL, NULL, sizeof(int), 4));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_ring_buffer_init(&rb, NULL, 0, 4));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_ring_buffer_init(&rb, NULL, sizeof(int), 0));
}

static void test_capacity_rounded_to_pow2(void)
{
    d4np_ring_buffer_t rb;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_ring_buffer_init(&rb, NULL, sizeof(int), 5));
    TEST_ASSERT_EQUAL_size_t(8, d4np_ring_buffer_capacity(&rb)); /* 5 -> 8 */
    d4np_ring_buffer_destroy(&rb);
}

static void test_push_pop_fifo_and_full_empty(void)
{
    d4np_ring_buffer_t rb;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_ring_buffer_init(&rb, NULL, sizeof(int), 4));
    TEST_ASSERT_TRUE(d4np_ring_buffer_is_empty(&rb));

    for (int i = 0; i < 4; ++i) {
        TEST_ASSERT_TRUE(d4np_ring_buffer_push(&rb, &i));
    }
    /* All four slots usable; the fifth push fails (full). */
    int extra = 99;
    TEST_ASSERT_FALSE(d4np_ring_buffer_push(&rb, &extra));
    TEST_ASSERT_EQUAL_size_t(4, d4np_ring_buffer_count(&rb));

    for (int i = 0; i < 4; ++i) {
        int out = -1;
        TEST_ASSERT_TRUE(d4np_ring_buffer_pop(&rb, &out));
        TEST_ASSERT_EQUAL_INT(i, out); /* FIFO order */
    }
    int sink = 0;
    TEST_ASSERT_FALSE(d4np_ring_buffer_pop(&rb, &sink)); /* empty */
    d4np_ring_buffer_destroy(&rb);
}

static void test_wraparound(void)
{
    d4np_ring_buffer_t rb;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_ring_buffer_init(&rb, NULL, sizeof(int), 4));
    /* Drive head/tail well past capacity to exercise index wrap. */
    for (int round = 0; round < 100; ++round) {
        int in = round;
        TEST_ASSERT_TRUE(d4np_ring_buffer_push(&rb, &in));
        int out = -1;
        TEST_ASSERT_TRUE(d4np_ring_buffer_pop(&rb, &out));
        TEST_ASSERT_EQUAL_INT(round, out);
    }
    TEST_ASSERT_TRUE(d4np_ring_buffer_is_empty(&rb));
    d4np_ring_buffer_destroy(&rb);
}

/* ---- multi-threaded SPSC run ------------------------------------------- */

enum { SPSC_COUNT = 200000 };

static void producer_run(void *arg)
{
    d4np_ring_buffer_t *rb = (d4np_ring_buffer_t *)arg;
    for (int i = 0; i < SPSC_COUNT; ++i) {
        while (!d4np_ring_buffer_push(rb, &i)) {
            /* spin until the consumer frees a slot */
        }
    }
}

static void test_spsc_producer_consumer(void)
{
    d4np_ring_buffer_t rb;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_ring_buffer_init(&rb, NULL, sizeof(int), 1024));

    test_thread_pack pack = {producer_run, &rb};
    test_thread_t producer = test_thread_spawn(&pack);

    /* Main thread is the single consumer; verify exact FIFO sequence 0..N-1. */
    int expected = 0;
    while (expected < SPSC_COUNT) {
        int out = -1;
        if (d4np_ring_buffer_pop(&rb, &out)) {
            TEST_ASSERT_EQUAL_INT(expected, out);
            ++expected;
        }
    }
    test_thread_join(producer);

    TEST_ASSERT_EQUAL_INT(SPSC_COUNT, expected);
    TEST_ASSERT_TRUE(d4np_ring_buffer_is_empty(&rb));
    d4np_ring_buffer_destroy(&rb);
}

void suite_ring_buffer(void)
{
    RUN_TEST(test_init_rejects_bad_args);
    RUN_TEST(test_capacity_rounded_to_pow2);
    RUN_TEST(test_push_pop_fifo_and_full_empty);
    RUN_TEST(test_wraparound);
    RUN_TEST(test_spsc_producer_consumer);
}
