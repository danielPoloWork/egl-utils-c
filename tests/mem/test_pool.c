/*
 * d4np-c — tests for the fixed-block pool allocator (#4-#5).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

#include <stdint.h>

static void test_init_rejects_bad_args(void)
{
    d4np_pool_t pool;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_pool_init(NULL, NULL, 32, 4));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_pool_init(&pool, NULL, 0, 4));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_pool_init(&pool, NULL, 32, 0));
}

static void test_alloc_until_exhausted_then_free(void)
{
    d4np_pool_t pool;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_pool_init(&pool, NULL, sizeof(int), 3));
    TEST_ASSERT_EQUAL_size_t(3, d4np_pool_available(&pool));

    void *a = d4np_pool_alloc(&pool);
    void *b = d4np_pool_alloc(&pool);
    void *c = d4np_pool_alloc(&pool);
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_size_t(0, d4np_pool_available(&pool));

    /* Exhausted pool yields NULL. */
    TEST_ASSERT_NULL(d4np_pool_alloc(&pool));

    /* Returned slots become available again and are reused (LIFO). */
    d4np_pool_free(&pool, b);
    TEST_ASSERT_EQUAL_size_t(1, d4np_pool_available(&pool));
    void *reused = d4np_pool_alloc(&pool);
    TEST_ASSERT_EQUAL_PTR(b, reused);

    d4np_pool_free(&pool, a);
    d4np_pool_free(&pool, c);
    d4np_pool_free(&pool, reused);
    TEST_ASSERT_EQUAL_size_t(3, d4np_pool_available(&pool));

    d4np_pool_destroy(&pool);
}

static void test_slots_are_distinct_and_writable(void)
{
    d4np_pool_t pool;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_pool_init(&pool, NULL, sizeof(int), 4));

    int *slots[4];
    for (int i = 0; i < 4; ++i) {
        slots[i] = d4np_pool_alloc(&pool);
        TEST_ASSERT_NOT_NULL(slots[i]);
        *slots[i] = i; /* must be independently writable */
    }
    for (int i = 0; i < 4; ++i) {
        TEST_ASSERT_EQUAL_INT(i, *slots[i]);
        for (int j = i + 1; j < 4; ++j) {
            TEST_ASSERT_NOT_EQUAL(slots[i], slots[j]);
        }
    }
    d4np_pool_destroy(&pool);
}

static void test_block_size_rounded_up_for_pointer(void)
{
    /* A 1-byte block must still be allocatable (rounded up to hold the free-list pointer). */
    d4np_pool_t pool;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_pool_init(&pool, NULL, 1, 2));
    void *a = d4np_pool_alloc(&pool);
    void *b = d4np_pool_alloc(&pool);
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_NOT_EQUAL(a, b);
    d4np_pool_destroy(&pool);
}

static void test_free_null_is_noop(void)
{
    d4np_pool_t pool;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_pool_init(&pool, NULL, 16, 2));
    d4np_pool_free(&pool, NULL); /* must not change state or crash */
    TEST_ASSERT_EQUAL_size_t(2, d4np_pool_available(&pool));
    d4np_pool_destroy(&pool);
}

void suite_pool(void)
{
    RUN_TEST(test_init_rejects_bad_args);
    RUN_TEST(test_alloc_until_exhausted_then_free);
    RUN_TEST(test_slots_are_distinct_and_writable);
    RUN_TEST(test_block_size_rounded_up_for_pointer);
    RUN_TEST(test_free_null_is_noop);
}
