/*
 * d4np-c — tests for the dynamic array (#6).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

static void test_init_rejects_bad_args(void)
{
    d4np_vector_t v;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_vector_init(NULL, NULL, 4, 0));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_vector_init(&v, NULL, 0, 0));
}

static void test_push_pop_at(void)
{
    d4np_vector_t v;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_vector_init(&v, NULL, sizeof(int), 0));
    TEST_ASSERT_TRUE(d4np_vector_is_empty(&v));

    for (int i = 0; i < 5; ++i) {
        TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_vector_push(&v, &i));
    }
    TEST_ASSERT_EQUAL_size_t(5, d4np_vector_len(&v));
    for (int i = 0; i < 5; ++i) {
        int *p = d4np_vector_at(&v, (size_t)i);
        TEST_ASSERT_NOT_NULL(p);
        TEST_ASSERT_EQUAL_INT(i, *p);
    }
    TEST_ASSERT_NULL(d4np_vector_at(&v, 5)); /* out of range */

    int out = -1;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_vector_pop(&v, &out));
    TEST_ASSERT_EQUAL_INT(4, out);
    TEST_ASSERT_EQUAL_size_t(4, d4np_vector_len(&v));

    d4np_vector_destroy(&v);
}

static void test_pop_empty_returns_not_found(void)
{
    d4np_vector_t v;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_vector_init(&v, NULL, sizeof(int), 0));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_NOT_FOUND, d4np_vector_pop(&v, NULL));
    d4np_vector_destroy(&v);
}

static void test_geometric_growth(void)
{
    d4np_vector_t v;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_vector_init(&v, NULL, sizeof(int), 0));

    size_t reallocations = 0;
    size_t last_cap = 0;
    for (int i = 0; i < 1000; ++i) {
        TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_vector_push(&v, &i));
        if (d4np_vector_capacity(&v) != last_cap) {
            last_cap = d4np_vector_capacity(&v);
            ++reallocations;
        }
    }
    TEST_ASSERT_EQUAL_size_t(1000, d4np_vector_len(&v));
    TEST_ASSERT_TRUE(d4np_vector_capacity(&v) >= 1000);
    /* Geometric (2x) growth keeps reallocations logarithmic, not linear. */
    TEST_ASSERT_TRUE_MESSAGE(reallocations < 20, "growth is not geometric");

    d4np_vector_destroy(&v);
}

static void test_clear_keeps_capacity(void)
{
    d4np_vector_t v;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_vector_init(&v, NULL, sizeof(int), 16));
    int x = 7;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_vector_push(&v, &x));
    size_t cap = d4np_vector_capacity(&v);
    d4np_vector_clear(&v);
    TEST_ASSERT_EQUAL_size_t(0, d4np_vector_len(&v));
    TEST_ASSERT_EQUAL_size_t(cap, d4np_vector_capacity(&v));
    d4np_vector_destroy(&v);
}

void suite_vector(void)
{
    RUN_TEST(test_init_rejects_bad_args);
    RUN_TEST(test_push_pop_at);
    RUN_TEST(test_pop_empty_returns_not_found);
    RUN_TEST(test_geometric_growth);
    RUN_TEST(test_clear_keeps_capacity);
}
