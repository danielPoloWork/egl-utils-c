/*
 * d4np-c — tests for the arena allocator (#1-#3).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

#include <stdint.h>

static void test_init_rejects_bad_args(void)
{
    d4np_arena_t arena;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_arena_init(NULL, NULL, 64));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_arena_init(&arena, NULL, 0));
}

static void test_alloc_is_sequential_and_bounded(void)
{
    d4np_arena_t arena;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_arena_init(&arena, NULL, 64));
    TEST_ASSERT_EQUAL_size_t(64, d4np_arena_remaining(&arena));

    unsigned char *a = d4np_arena_alloc(&arena, 16, 1);
    unsigned char *b = d4np_arena_alloc(&arena, 16, 1);
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_EQUAL_PTR(a + 16, b); /* bump, contiguous */
    TEST_ASSERT_EQUAL_size_t(32, d4np_arena_used(&arena));

    /* Exhaustion returns NULL without corrupting state. */
    TEST_ASSERT_NULL(d4np_arena_alloc(&arena, 64, 1));
    TEST_ASSERT_EQUAL_size_t(32, d4np_arena_used(&arena));

    d4np_arena_destroy(&arena);
}

static void test_alloc_honors_alignment(void)
{
    d4np_arena_t arena;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_arena_init(&arena, NULL, 256));

    (void)d4np_arena_alloc(&arena, 1, 1); /* offset now 1, misaligned */
    void *p = d4np_arena_alloc(&arena, 8, 16);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)((uintptr_t)p % 16));

    d4np_arena_destroy(&arena);
}

static void test_reset_reuses_capacity(void)
{
    d4np_arena_t arena;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_arena_init(&arena, NULL, 64));

    void *first = d4np_arena_alloc(&arena, 64, 1);
    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_EQUAL_size_t(0, d4np_arena_remaining(&arena));

    d4np_arena_reset(&arena);
    TEST_ASSERT_EQUAL_size_t(64, d4np_arena_remaining(&arena));

    void *again = d4np_arena_alloc(&arena, 64, 1);
    TEST_ASSERT_EQUAL_PTR(first, again); /* same block reused */

    d4np_arena_destroy(&arena);
}

static void test_zero_size_alloc_is_null(void)
{
    d4np_arena_t arena;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_arena_init(&arena, NULL, 32));
    TEST_ASSERT_NULL(d4np_arena_alloc(&arena, 0, 8));
    d4np_arena_destroy(&arena);
}

void suite_arena(void)
{
    RUN_TEST(test_init_rejects_bad_args);
    RUN_TEST(test_alloc_is_sequential_and_bounded);
    RUN_TEST(test_alloc_honors_alignment);
    RUN_TEST(test_reset_reuses_capacity);
    RUN_TEST(test_zero_size_alloc_is_null);
}
