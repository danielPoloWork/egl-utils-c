/*
 * d4np-c — tests for path joining (#20).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

#include <string.h>

#if defined(_WIN32)
#define S "\\"
#else
#define S "/"
#endif

static void expect_combine(const char *a, const char *b, const char *expected)
{
    char out[64];
    size_t n = d4np_path_combine(out, sizeof(out), a, b);
    TEST_ASSERT_EQUAL_size_t(strlen(expected), n);
    TEST_ASSERT_EQUAL_STRING(expected, out);
}

static void test_basic_join(void)
{
    expect_combine("dir", "file", "dir" S "file");
}

static void test_dedup_separators(void)
{
    expect_combine("dir" S, "file", "dir" S "file");
    expect_combine("dir", S "file", "dir" S "file");
    expect_combine("dir" S, S "file", "dir" S "file");
}

static void test_empty_segments(void)
{
    expect_combine("", "file", "file");
    expect_combine("dir", "", "dir");
    expect_combine("", "", "");
}

static void test_root_left(void)
{
    expect_combine(S, "x", S "x");
}

static void test_truncation_reports_full_length(void)
{
    char small[4];
    size_t n = d4np_path_combine(small, sizeof(small), "abc", "def"); /* "abc/def" = 7 */
    TEST_ASSERT_EQUAL_size_t(7, n);                                   /* full length reported */
    TEST_ASSERT_EQUAL_size_t(3, strlen(small));                       /* truncated + NUL-terminated */

    /* Sizing pass: out_size 0 writes nothing, still returns the needed length. */
    TEST_ASSERT_EQUAL_size_t(7, d4np_path_combine(NULL, 0, "abc", "def"));
}

void suite_path(void)
{
    RUN_TEST(test_basic_join);
    RUN_TEST(test_dedup_separators);
    RUN_TEST(test_empty_segments);
    RUN_TEST(test_root_left);
    RUN_TEST(test_truncation_reports_full_length);
}
