/*
 * d4np-c — tests for the string builder (#10).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

#include <string.h>

static void test_empty_builder_is_empty_string(void)
{
    d4np_string_builder_t sb;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_init(&sb, NULL, 0));
    TEST_ASSERT_EQUAL_size_t(0, d4np_sb_len(&sb));
    TEST_ASSERT_EQUAL_STRING("", d4np_sb_cstr(&sb));
    d4np_sb_destroy(&sb);
}

static void test_append_builds_string(void)
{
    d4np_string_builder_t sb;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_init(&sb, NULL, 0));
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_append(&sb, "Hello"));
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_append_char(&sb, ' '));
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_append(&sb, "World"));
    TEST_ASSERT_EQUAL_STRING("Hello World", d4np_sb_cstr(&sb));
    TEST_ASSERT_EQUAL_size_t(11, d4np_sb_len(&sb));
    d4np_sb_destroy(&sb);
}

static void test_append_view(void)
{
    d4np_string_builder_t sb;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_init(&sb, NULL, 0));
    d4np_str_view_t v = d4np_str_view_from_str("Daniel;Polo");
    d4np_str_view_t tok;
    while (d4np_str_view_split_next(&v, ';', &tok)) {
        TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_append_view(&sb, tok));
    }
    TEST_ASSERT_EQUAL_STRING("DanielPolo", d4np_sb_cstr(&sb));
    d4np_sb_destroy(&sb);
}

static void test_many_appends_grow_correctly(void)
{
    d4np_string_builder_t sb;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_init(&sb, NULL, 0));
    for (int i = 0; i < 1000; ++i) {
        TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_append(&sb, "ab"));
    }
    TEST_ASSERT_EQUAL_size_t(2000, d4np_sb_len(&sb));
    TEST_ASSERT_EQUAL_size_t(2000, strlen(d4np_sb_cstr(&sb)));
    /* spot-check content */
    TEST_ASSERT_EQUAL_CHAR('a', d4np_sb_cstr(&sb)[1998]);
    TEST_ASSERT_EQUAL_CHAR('b', d4np_sb_cstr(&sb)[1999]);
    d4np_sb_destroy(&sb);
}

static void test_view_and_clear(void)
{
    d4np_string_builder_t sb;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_init(&sb, NULL, 8));
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_append(&sb, "abc"));

    d4np_str_view_t view = d4np_sb_view(&sb);
    TEST_ASSERT_EQUAL_size_t(3, view.len);
    TEST_ASSERT_TRUE(d4np_str_view_equals(view, d4np_str_view_from_str("abc")));

    d4np_sb_clear(&sb);
    TEST_ASSERT_EQUAL_size_t(0, d4np_sb_len(&sb));
    TEST_ASSERT_EQUAL_STRING("", d4np_sb_cstr(&sb));
    /* reusable after clear */
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_sb_append(&sb, "xy"));
    TEST_ASSERT_EQUAL_STRING("xy", d4np_sb_cstr(&sb));
    d4np_sb_destroy(&sb);
}

void suite_string_builder(void)
{
    RUN_TEST(test_empty_builder_is_empty_string);
    RUN_TEST(test_append_builds_string);
    RUN_TEST(test_append_view);
    RUN_TEST(test_many_appends_grow_correctly);
    RUN_TEST(test_view_and_clear);
}
