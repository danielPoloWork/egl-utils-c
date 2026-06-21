/*
 * d4np-c — tests for string-to-number parsing (#17).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

static int64_t parse_int_ok(const char *s, int base)
{
    int64_t out = 0;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_str_parse_int(d4np_str_view_from_str(s), base, &out));
    return out;
}

static void test_parse_int_basic(void)
{
    TEST_ASSERT_EQUAL_INT64(0, parse_int_ok("0", 10));
    TEST_ASSERT_EQUAL_INT64(42, parse_int_ok("42", 10));
    TEST_ASSERT_EQUAL_INT64(-7, parse_int_ok("-7", 10));
    TEST_ASSERT_EQUAL_INT64(7, parse_int_ok("+7", 10));
    TEST_ASSERT_EQUAL_INT64(255, parse_int_ok("  255  ", 10)); /* surrounding spaces ok */
}

static void test_parse_int_bases(void)
{
    TEST_ASSERT_EQUAL_INT64(255, parse_int_ok("ff", 16));
    TEST_ASSERT_EQUAL_INT64(255, parse_int_ok("0xFF", 16));
    TEST_ASSERT_EQUAL_INT64(255, parse_int_ok("0xff", 0)); /* auto-detect hex */
    TEST_ASSERT_EQUAL_INT64(8, parse_int_ok("010", 0));    /* auto-detect octal */
    TEST_ASSERT_EQUAL_INT64(5, parse_int_ok("101", 2));    /* binary */
}

static void test_parse_int_limits(void)
{
    TEST_ASSERT_EQUAL_INT64(INT64_MAX, parse_int_ok("9223372036854775807", 10));
    TEST_ASSERT_EQUAL_INT64(INT64_MIN, parse_int_ok("-9223372036854775808", 10));
}

static void test_parse_int_errors(void)
{
    int64_t out = 123;
    /* overflow just past INT64_MAX */
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_OVERFLOW,
                          d4np_str_parse_int(d4np_str_view_from_str("9223372036854775808"), 10, &out));
    /* trailing garbage */
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_str_parse_int(d4np_str_view_from_str("12x"), 10, &out));
    /* no digits */
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_str_parse_int(d4np_str_view_from_str("   "), 10, &out));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_str_parse_int(d4np_str_view_from_str("+"), 10, &out));
    /* bad base / NULL out */
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_str_parse_int(d4np_str_view_from_str("1"), 99, &out));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_str_parse_int(d4np_str_view_from_str("1"), 10, NULL));
}

static void test_parse_float_basic(void)
{
    double out = 0.0;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_str_parse_float(d4np_str_view_from_str("3.14"), &out));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 3.14, out);
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_str_parse_float(d4np_str_view_from_str("  -2.5e3 "), &out));
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, -2500.0, out);
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_str_parse_float(d4np_str_view_from_str("0"), &out));
    TEST_ASSERT_DOUBLE_WITHIN(1e-12, 0.0, out);
}

static void test_parse_float_errors(void)
{
    double out = 1.0;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_str_parse_float(d4np_str_view_from_str("1.0q"), &out));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_str_parse_float(d4np_str_view_from_str(""), &out));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_str_parse_float(d4np_str_view_from_str("  "), &out));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_OVERFLOW, d4np_str_parse_float(d4np_str_view_from_str("1e400"), &out));
}

void suite_parse(void)
{
    RUN_TEST(test_parse_int_basic);
    RUN_TEST(test_parse_int_bases);
    RUN_TEST(test_parse_int_limits);
    RUN_TEST(test_parse_int_errors);
    RUN_TEST(test_parse_float_basic);
    RUN_TEST(test_parse_float_errors);
}
