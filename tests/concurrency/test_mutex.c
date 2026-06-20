/*
 * d4np-c — tests for the portable mutex (#11).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"
#include "test_threads.h"

static void test_lock_unlock_trylock(void)
{
    d4np_mutex_t m;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_mutex_init(&m));

    /* Basic lock/unlock cycle. */
    d4np_mutex_lock(&m);
    d4np_mutex_unlock(&m);

    /* trylock on a free mutex acquires it; release it again. (We do not probe same-thread
     * re-locking: recursion is left undefined by the contract — see mutex.h.) */
    TEST_ASSERT_TRUE(d4np_mutex_trylock(&m));
    d4np_mutex_unlock(&m);

    d4np_mutex_destroy(&m);
}

typedef struct {
    d4np_mutex_t *mutex;
    long *counter;
    int iterations;
} incr_arg;

static void increment_worker(void *arg)
{
    incr_arg *a = (incr_arg *)arg;
    for (int i = 0; i < a->iterations; ++i) {
        d4np_mutex_lock(a->mutex);
        ++(*a->counter);
        d4np_mutex_unlock(a->mutex);
    }
}

static void test_mutual_exclusion_under_contention(void)
{
    enum { THREADS = 4, ITERS = 100000 };
    d4np_mutex_t mutex;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_mutex_init(&mutex));

    long counter = 0;
    incr_arg arg = {&mutex, &counter, ITERS};
    test_thread_pack packs[THREADS];
    test_thread_t threads[THREADS];
    for (int i = 0; i < THREADS; ++i) {
        packs[i].fn = increment_worker;
        packs[i].arg = &arg;
        threads[i] = test_thread_spawn(&packs[i]);
    }
    for (int i = 0; i < THREADS; ++i) {
        test_thread_join(threads[i]);
    }

    /* Without the lock this count would race and come up short. */
    TEST_ASSERT_EQUAL_INT64((long)THREADS * ITERS, counter);
    d4np_mutex_destroy(&mutex);
}

void suite_mutex(void)
{
    RUN_TEST(test_lock_unlock_trylock);
    RUN_TEST(test_mutual_exclusion_under_contention);
}
