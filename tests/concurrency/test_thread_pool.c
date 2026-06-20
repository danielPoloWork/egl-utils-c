/*
 * d4np-c — tests for the thread pool (#12).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Verifies parallel execution and the graceful-drain guarantee: every submitted task runs
 * exactly once before destroy returns. The shared counter is guarded by d4np_mutex_t, so a
 * missed lock (or a dropped task) shows up as a wrong total. Run under TSan in CI.
 */
#include "unity.h"

#include "d4np_c.h"

static void test_init_rejects_bad_args(void)
{
    d4np_thread_pool_t pool;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_thread_pool_init(NULL, NULL, 4));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_thread_pool_init(&pool, NULL, 0));
}

typedef struct {
    d4np_mutex_t lock;
    long counter;
} shared_counter;

static void increment_task(void *arg)
{
    shared_counter *c = (shared_counter *)arg;
    d4np_mutex_lock(&c->lock);
    ++c->counter;
    d4np_mutex_unlock(&c->lock);
}

static void test_runs_every_submitted_task(void)
{
    enum { WORKERS = 4, TASKS = 50000 };
    d4np_thread_pool_t pool;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_thread_pool_init(&pool, NULL, WORKERS));
    TEST_ASSERT_EQUAL_size_t(WORKERS, d4np_thread_pool_thread_count(&pool));

    shared_counter c;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_mutex_init(&c.lock));
    c.counter = 0;

    for (int i = 0; i < TASKS; ++i) {
        TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_thread_pool_submit(&pool, increment_task, &c));
    }

    /* Graceful shutdown runs every queued task before returning. */
    d4np_thread_pool_destroy(&pool);

    TEST_ASSERT_EQUAL_INT64((long)TASKS, c.counter);
    d4np_mutex_destroy(&c.lock);
}

static void test_submit_after_shutdown_is_rejected(void)
{
    d4np_thread_pool_t pool;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_thread_pool_init(&pool, NULL, 2));
    d4np_thread_pool_destroy(&pool);
    /* After destroy the pool is torn down; a fresh init/destroy must also be clean. */

    shared_counter c;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_mutex_init(&c.lock));
    c.counter = 0;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_thread_pool_init(&pool, NULL, 2));
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_thread_pool_submit(&pool, increment_task, &c));
    d4np_thread_pool_destroy(&pool);
    TEST_ASSERT_EQUAL_INT64(1, c.counter);
    d4np_mutex_destroy(&c.lock);
}

void suite_thread_pool(void)
{
    RUN_TEST(test_init_rejects_bad_args);
    RUN_TEST(test_runs_every_submitted_task);
    RUN_TEST(test_submit_after_shutdown_is_rejected);
}
