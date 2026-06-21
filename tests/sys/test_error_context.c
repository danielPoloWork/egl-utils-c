/*
 * d4np-c — tests for the thread-local error context (#22).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

#include <string.h>

static void test_push_pop_lifo(void)
{
    d4np_error_context_clear();
    TEST_ASSERT_EQUAL_size_t(0, d4np_error_context_depth());

    /* push returns the status unchanged (composes with `return`) */
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_IO, d4np_error_context_push(D4NP_ERR_IO, "open failed"));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INTERNAL, d4np_error_context_push(D4NP_ERR_INTERNAL, "startup failed"));
    TEST_ASSERT_EQUAL_size_t(2, d4np_error_context_depth());

    d4np_status_t st = D4NP_OK;
    const char *msg = NULL;
    TEST_ASSERT_TRUE(d4np_error_context_pop(&st, &msg)); /* LIFO: last pushed first */
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INTERNAL, st);
    TEST_ASSERT_EQUAL_STRING("startup failed", msg);

    TEST_ASSERT_TRUE(d4np_error_context_pop(&st, &msg));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_IO, st);
    TEST_ASSERT_EQUAL_STRING("open failed", msg);

    TEST_ASSERT_FALSE(d4np_error_context_pop(&st, &msg)); /* empty */
    TEST_ASSERT_EQUAL_size_t(0, d4np_error_context_depth());
}

static void test_clear_and_null_message(void)
{
    d4np_error_context_clear();
    (void)d4np_error_context_push(D4NP_ERR_NOT_FOUND, NULL);
    d4np_status_t st = D4NP_OK;
    const char *msg = NULL;
    TEST_ASSERT_TRUE(d4np_error_context_pop(&st, &msg));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_NOT_FOUND, st);
    TEST_ASSERT_EQUAL_STRING("", msg); /* NULL message -> empty string */

    (void)d4np_error_context_push(D4NP_ERR_IO, "x");
    (void)d4np_error_context_push(D4NP_ERR_IO, "y");
    d4np_error_context_clear();
    TEST_ASSERT_EQUAL_size_t(0, d4np_error_context_depth());
}

static void test_long_message_truncated(void)
{
    d4np_error_context_clear();
    char big[1024];
    memset(big, 'A', sizeof(big));
    big[sizeof(big) - 1] = '\0';
    (void)d4np_error_context_push(D4NP_ERR_INTERNAL, big);

    const char *msg = NULL;
    TEST_ASSERT_TRUE(d4np_error_context_pop(NULL, &msg));
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_TRUE(strlen(msg) < sizeof(big)); /* stored copy is bounded */
    TEST_ASSERT_EQUAL_CHAR('A', msg[0]);
}

void suite_error_context(void)
{
    RUN_TEST(test_push_pop_lifo);
    RUN_TEST(test_clear_and_null_message);
    RUN_TEST(test_long_message_truncated);
}
