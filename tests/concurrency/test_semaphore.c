/*
 * d4np-c — tests for the counting semaphore (#14).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"
#include "test_threads.h"

static void test_trywait_respects_count(void)
{
    d4np_semaphore_t s;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_semaphore_init(&s, 2));

    TEST_ASSERT_TRUE(d4np_semaphore_trywait(&s));  /* 2 -> 1 */
    TEST_ASSERT_TRUE(d4np_semaphore_trywait(&s));  /* 1 -> 0 */
    TEST_ASSERT_FALSE(d4np_semaphore_trywait(&s)); /* 0: no permit */

    d4np_semaphore_post(&s);                       /* 0 -> 1 */
    TEST_ASSERT_TRUE(d4np_semaphore_trywait(&s));  /* 1 -> 0 */

    d4np_semaphore_destroy(&s);
}

typedef struct {
    d4np_semaphore_t *sem;
    int *shared;
} handoff_arg;

static void poster_worker(void *arg)
{
    handoff_arg *a = (handoff_arg *)arg;
    *a->shared = 42;            /* produce a value ... */
    d4np_semaphore_post(a->sem); /* ... then publish it */
}

static void test_wait_post_handoff(void)
{
    /* A zero-count semaphore makes wait() block until the worker posts, establishing a
     * happens-before edge so the consumer observes the produced value. */
    d4np_semaphore_t sem;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_semaphore_init(&sem, 0));

    int shared = 0;
    handoff_arg arg = {&sem, &shared};
    test_thread_pack pack = {poster_worker, &arg};
    test_thread_t worker = test_thread_spawn(&pack);

    d4np_semaphore_wait(&sem); /* blocks until poster_worker posts */
    TEST_ASSERT_EQUAL_INT(42, shared);

    test_thread_join(worker);
    d4np_semaphore_destroy(&sem);
}

static void test_counts_resources(void)
{
    /* N permits => exactly N concurrent acquisitions before blocking. */
    enum { PERMITS = 3 };
    d4np_semaphore_t sem;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_semaphore_init(&sem, PERMITS));
    int acquired = 0;
    while (d4np_semaphore_trywait(&sem)) {
        ++acquired;
    }
    TEST_ASSERT_EQUAL_INT(PERMITS, acquired);
    d4np_semaphore_destroy(&sem);
}

void suite_semaphore(void)
{
    RUN_TEST(test_trywait_respects_count);
    RUN_TEST(test_wait_post_handoff);
    RUN_TEST(test_counts_resources);
}
