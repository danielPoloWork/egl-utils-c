/*
 * d4np-c — tests for the hash map (#7).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

static void test_init_rejects_bad_args(void)
{
    d4np_hashmap_t m;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT,
                          d4np_hashmap_init(NULL, NULL, sizeof(int), sizeof(int), 0, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT,
                          d4np_hashmap_init(&m, NULL, 0, sizeof(int), 0, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT,
                          d4np_hashmap_init(&m, NULL, sizeof(int), 0, 0, NULL, NULL));
}

static void test_put_get_overwrite(void)
{
    d4np_hashmap_t m;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_init(&m, NULL, sizeof(int), sizeof(int), 0, NULL, NULL));

    int k = 42, v = 100;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_put(&m, &k, &v));
    TEST_ASSERT_EQUAL_size_t(1, d4np_hashmap_len(&m));

    int *got = d4np_hashmap_get(&m, &k);
    TEST_ASSERT_NOT_NULL(got);
    TEST_ASSERT_EQUAL_INT(100, *got);

    v = 200;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_put(&m, &k, &v)); /* overwrite */
    TEST_ASSERT_EQUAL_size_t(1, d4np_hashmap_len(&m));            /* not a new entry */
    got = d4np_hashmap_get(&m, &k);
    TEST_ASSERT_EQUAL_INT(200, *got);

    int missing = 7;
    TEST_ASSERT_NULL(d4np_hashmap_get(&m, &missing));
    TEST_ASSERT_FALSE(d4np_hashmap_contains(&m, &missing));

    d4np_hashmap_destroy(&m);
}

static void test_many_entries_and_growth(void)
{
    d4np_hashmap_t m;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_init(&m, NULL, sizeof(int), sizeof(int), 0, NULL, NULL));

    for (int i = 0; i < 1000; ++i) {
        int v = i * 3;
        TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_put(&m, &i, &v));
    }
    TEST_ASSERT_EQUAL_size_t(1000, d4np_hashmap_len(&m));

    for (int i = 0; i < 1000; ++i) {
        int *got = d4np_hashmap_get(&m, &i);
        TEST_ASSERT_NOT_NULL(got);
        TEST_ASSERT_EQUAL_INT(i * 3, *got);
    }
    d4np_hashmap_destroy(&m);
}

static void test_remove_and_tombstone_reuse(void)
{
    d4np_hashmap_t m;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_init(&m, NULL, sizeof(int), sizeof(int), 16, NULL, NULL));

    for (int i = 0; i < 10; ++i) {
        int v = i;
        TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_put(&m, &i, &v));
    }
    int four = 4;
    TEST_ASSERT_TRUE(d4np_hashmap_remove(&m, &four));
    TEST_ASSERT_FALSE(d4np_hashmap_remove(&m, &four)); /* already gone */
    TEST_ASSERT_EQUAL_size_t(9, d4np_hashmap_len(&m));
    TEST_ASSERT_NULL(d4np_hashmap_get(&m, &four));

    /* Re-insert reuses the slot; other keys still reachable across the tombstone. */
    int v = 444;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_put(&m, &four, &v));
    TEST_ASSERT_EQUAL_INT(444, *(int *)d4np_hashmap_get(&m, &four));
    for (int i = 0; i < 10; ++i) {
        TEST_ASSERT_TRUE(d4np_hashmap_contains(&m, &i));
    }
    d4np_hashmap_destroy(&m);
}

static void test_clear_keeps_capacity(void)
{
    d4np_hashmap_t m;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_init(&m, NULL, sizeof(int), sizeof(int), 32, NULL, NULL));
    for (int i = 0; i < 5; ++i) {
        TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_put(&m, &i, &i));
    }
    d4np_hashmap_clear(&m);
    TEST_ASSERT_EQUAL_size_t(0, d4np_hashmap_len(&m));
    int zero = 0;
    TEST_ASSERT_NULL(d4np_hashmap_get(&m, &zero));
    /* reusable */
    int v = 9;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_hashmap_put(&m, &zero, &v));
    TEST_ASSERT_EQUAL_INT(9, *(int *)d4np_hashmap_get(&m, &zero));
    d4np_hashmap_destroy(&m);
}

void suite_hashmap(void)
{
    RUN_TEST(test_init_rejects_bad_args);
    RUN_TEST(test_put_get_overwrite);
    RUN_TEST(test_many_entries_and_growth);
    RUN_TEST(test_remove_and_tombstone_reuse);
    RUN_TEST(test_clear_keeps_capacity);
}
