/*
 * d4np-c — unit tests for the foundation + str_view slice (Milestone 1).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static void expect_view(d4np_str_view_t v, const char *literal)
{
    TEST_ASSERT_EQUAL_size_t(strlen(literal), v.len);
    if (v.len > 0) {
        TEST_ASSERT_EQUAL_CHAR_ARRAY(literal, v.ptr, v.len);
    }
}

static void test_from_str_basic(void)
{
    d4np_str_view_t sv = d4np_str_view_from_str("hello");
    expect_view(sv, "hello");
}

static void test_from_str_null_is_empty(void)
{
    d4np_str_view_t sv = d4np_str_view_from_str(NULL);
    TEST_ASSERT_NULL(sv.ptr);
    TEST_ASSERT_EQUAL_size_t(0, sv.len);
}

static void test_equals(void)
{
    TEST_ASSERT_TRUE(d4np_str_view_equals(d4np_str_view_from_str("abc"), d4np_str_view_from_str("abc")));
    TEST_ASSERT_FALSE(d4np_str_view_equals(d4np_str_view_from_str("abc"), d4np_str_view_from_str("abd")));
    TEST_ASSERT_FALSE(d4np_str_view_equals(d4np_str_view_from_str("ab"), d4np_str_view_from_str("abc")));
    TEST_ASSERT_TRUE(d4np_str_view_equals(d4np_str_view_from_str(""), d4np_str_view_from_str(NULL)));
}

static void test_split_spec_example(void)
{
    /* The exact example from the spec (§3). */
    d4np_str_view_t sv = d4np_str_view_from_str("Daniel;Polo;Architect");
    d4np_str_view_t tok;
    const char *expect[] = {"Daniel", "Polo", "Architect"};
    int n = 0;
    while (d4np_str_view_split_next(&sv, ';', &tok)) {
        TEST_ASSERT_TRUE_MESSAGE(n < 3, "produced more tokens than expected");
        expect_view(tok, expect[n]);
        ++n;
    }
    TEST_ASSERT_EQUAL_INT(3, n);
}

static void test_split_adjacent_separators_yield_empty_fields(void)
{
    d4np_str_view_t sv = d4np_str_view_from_str("a;;b");
    d4np_str_view_t tok;
    const char *expect[] = {"a", "", "b"};
    int n = 0;
    while (d4np_str_view_split_next(&sv, ';', &tok)) {
        expect_view(tok, expect[n]);
        ++n;
    }
    TEST_ASSERT_EQUAL_INT(3, n);
}

static void test_split_trailing_separator(void)
{
    d4np_str_view_t sv = d4np_str_view_from_str("a;");
    d4np_str_view_t tok;
    TEST_ASSERT_TRUE(d4np_str_view_split_next(&sv, ';', &tok));
    expect_view(tok, "a");
    TEST_ASSERT_TRUE(d4np_str_view_split_next(&sv, ';', &tok));
    expect_view(tok, "");
    TEST_ASSERT_FALSE(d4np_str_view_split_next(&sv, ';', &tok));
}

static void test_split_empty_view_yields_one_empty_field(void)
{
    d4np_str_view_t sv = d4np_str_view_from_str("");
    d4np_str_view_t tok;
    TEST_ASSERT_TRUE(d4np_str_view_split_next(&sv, ';', &tok));
    expect_view(tok, "");
    TEST_ASSERT_FALSE(d4np_str_view_split_next(&sv, ';', &tok));
}

static void test_split_no_separator(void)
{
    d4np_str_view_t sv = d4np_str_view_from_str("whole");
    d4np_str_view_t tok;
    TEST_ASSERT_TRUE(d4np_str_view_split_next(&sv, ';', &tok));
    expect_view(tok, "whole");
    TEST_ASSERT_FALSE(d4np_str_view_split_next(&sv, ';', &tok));
}

static void test_allocator_default_roundtrip(void)
{
    const d4np_allocator_t *a = d4np_allocator_default();
    TEST_ASSERT_NOT_NULL(a);

    void *p = d4np_alloc(a, 128, sizeof(void *));
    TEST_ASSERT_NOT_NULL(p);
    p = d4np_realloc(a, p, 128, 256, sizeof(void *));
    TEST_ASSERT_NOT_NULL(p);
    d4np_free(a, p, 256);

    /* NULL allocator falls back to the default; zero size yields NULL. */
    TEST_ASSERT_NULL(d4np_alloc(NULL, 0, 0));
    d4np_free(NULL, NULL, 0); /* no-op, must not crash */
}

static void test_status_str(void)
{
    TEST_ASSERT_EQUAL_STRING("D4NP_OK", d4np_status_str(D4NP_OK));
    TEST_ASSERT_EQUAL_STRING("D4NP_ERR_OUT_OF_MEMORY", d4np_status_str(D4NP_ERR_OUT_OF_MEMORY));
    TEST_ASSERT_EQUAL_STRING("D4NP_ERR_UNKNOWN", d4np_status_str((d4np_status_t)9999));
}

static void test_version_string(void)
{
    TEST_ASSERT_EQUAL_STRING("0.0.0", D4NP_VERSION_STRING);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_from_str_basic);
    RUN_TEST(test_from_str_null_is_empty);
    RUN_TEST(test_equals);
    RUN_TEST(test_split_spec_example);
    RUN_TEST(test_split_adjacent_separators_yield_empty_fields);
    RUN_TEST(test_split_trailing_separator);
    RUN_TEST(test_split_empty_view_yields_one_empty_field);
    RUN_TEST(test_split_no_separator);
    RUN_TEST(test_allocator_default_roundtrip);
    RUN_TEST(test_status_str);
    RUN_TEST(test_version_string);
    return UNITY_END();
}
