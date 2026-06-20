/*
 * d4np-c — tests for the lock-free SPSC unbounded queue (#13).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Includes a real multi-threaded run (one producer thread, the main thread as consumer),
 * verifying exact FIFO delivery. Exercised under ThreadSanitizer in CI (ctest --preset tsan).
 */
#include "unity.h"

#include "d4np_c.h"
#include "test_threads.h"

static void test_init_rejects_bad_args(void)
{
    d4np_atomic_queue_t q;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_atomic_queue_init(NULL, NULL, sizeof(int)));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_atomic_queue_init(&q, NULL, 0));
}

static void test_fifo_single_thread(void)
{
    d4np_atomic_queue_t q;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_atomic_queue_init(&q, NULL, sizeof(int)));
    TEST_ASSERT_TRUE(d4np_atomic_queue_is_empty(&q));

    for (int i = 0; i < 5; ++i) {
        TEST_ASSERT_TRUE(d4np_atomic_queue_enqueue(&q, &i));
    }
    TEST_ASSERT_FALSE(d4np_atomic_queue_is_empty(&q));

    for (int i = 0; i < 5; ++i) {
        int out = -1;
        TEST_ASSERT_TRUE(d4np_atomic_queue_dequeue(&q, &out));
        TEST_ASSERT_EQUAL_INT(i, out); /* FIFO */
    }
    int sink = 0;
    TEST_ASSERT_FALSE(d4np_atomic_queue_dequeue(&q, &sink)); /* empty */
    TEST_ASSERT_TRUE(d4np_atomic_queue_is_empty(&q));

    d4np_atomic_queue_destroy(&q);
}

static void test_destroy_frees_pending_nodes(void)
{
    /* Leave items queued at destroy; Valgrind/ASan in CI prove nothing leaks. */
    d4np_atomic_queue_t q;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_atomic_queue_init(&q, NULL, sizeof(int)));
    for (int i = 0; i < 100; ++i) {
        TEST_ASSERT_TRUE(d4np_atomic_queue_enqueue(&q, &i));
    }
    d4np_atomic_queue_destroy(&q);
}

enum { SPSC_COUNT = 200000 };

static void producer_run(void *arg)
{
    d4np_atomic_queue_t *q = (d4np_atomic_queue_t *)arg;
    for (int i = 0; i < SPSC_COUNT; ++i) {
        while (!d4np_atomic_queue_enqueue(q, &i)) {
            /* retry only on OOM, which should not happen here */
        }
    }
}

static void test_spsc_producer_consumer(void)
{
    d4np_atomic_queue_t q;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_atomic_queue_init(&q, NULL, sizeof(int)));

    test_thread_pack pack = {producer_run, &q};
    test_thread_t producer = test_thread_spawn(&pack);

    int expected = 0;
    while (expected < SPSC_COUNT) {
        int out = -1;
        if (d4np_atomic_queue_dequeue(&q, &out)) {
            TEST_ASSERT_EQUAL_INT(expected, out);
            ++expected;
        }
    }
    test_thread_join(producer);

    TEST_ASSERT_EQUAL_INT(SPSC_COUNT, expected);
    TEST_ASSERT_TRUE(d4np_atomic_queue_is_empty(&q));
    d4np_atomic_queue_destroy(&q);
}

void suite_atomic_queue(void)
{
    RUN_TEST(test_init_rejects_bad_args);
    RUN_TEST(test_fifo_single_thread);
    RUN_TEST(test_destroy_frees_pending_nodes);
    RUN_TEST(test_spsc_producer_consumer);
}
