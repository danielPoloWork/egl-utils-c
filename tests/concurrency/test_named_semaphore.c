/*
 * d4np-c — single-process tests for the named/IPC semaphore (#14, M8.5).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Cross-process behavior is exercised by the separate multi-process harness
 * (named_sem_harness.c / the `d4np_named_sem_ipc` CTest). These checks cover the API contract
 * in-process and avoid blocking wait() (which needs a second actor).
 */
#include "unity.h"

#include "d4np_c.h"

#define NS_NAME "d4np_ut_nsem"

static void test_open_counts_and_trywait(void)
{
    (void)d4np_named_semaphore_unlink(NS_NAME); /* clear any stale object from a prior run */

    d4np_named_semaphore_t s;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_named_semaphore_open(&s, NS_NAME, 2));

    TEST_ASSERT_TRUE(d4np_named_semaphore_trywait(&s));  /* 2 -> 1 */
    TEST_ASSERT_TRUE(d4np_named_semaphore_trywait(&s));  /* 1 -> 0 */
    TEST_ASSERT_FALSE(d4np_named_semaphore_trywait(&s)); /* 0: no permit */

    d4np_named_semaphore_post(&s);                      /* 0 -> 1 */
    TEST_ASSERT_TRUE(d4np_named_semaphore_trywait(&s)); /* 1 -> 0 */

    d4np_named_semaphore_close(&s);
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_named_semaphore_unlink(NS_NAME));
}

static void test_two_handles_share_one_object(void)
{
    (void)d4np_named_semaphore_unlink(NS_NAME);

    /* The first open creates the object with 1 permit; the second attaches to the same name. */
    d4np_named_semaphore_t a, b;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_named_semaphore_open(&a, NS_NAME, 1));
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_named_semaphore_open(&b, NS_NAME, 0));

    /* A permit posted through one handle is visible through the other (one shared object). */
    TEST_ASSERT_TRUE(d4np_named_semaphore_trywait(&b));  /* consumes the initial permit */
    TEST_ASSERT_FALSE(d4np_named_semaphore_trywait(&a)); /* none left */
    d4np_named_semaphore_post(&a);
    TEST_ASSERT_TRUE(d4np_named_semaphore_trywait(&b));

    d4np_named_semaphore_close(&b);
    d4np_named_semaphore_close(&a);
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_named_semaphore_unlink(NS_NAME));
}

static void test_open_rejects_bad_arguments(void)
{
    d4np_named_semaphore_t s;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_named_semaphore_open(NULL, NS_NAME, 0));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_named_semaphore_open(&s, NULL, 0));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_named_semaphore_open(&s, "", 0));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_named_semaphore_unlink(NULL));
}

void suite_named_semaphore(void)
{
    RUN_TEST(test_open_counts_and_trywait);
    RUN_TEST(test_two_handles_share_one_object);
    RUN_TEST(test_open_rejects_bad_arguments);
}
