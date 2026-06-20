/*
 * d4np-c — tests for version-4 UUID generation (#24).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

#include <ctype.h>
#include <string.h>

static void test_version_and_variant_bits(void)
{
    d4np_uuid_t u;
    d4np_uuid_generate(&u);
    /* version 4 in the high nibble of byte 6 */
    TEST_ASSERT_EQUAL_UINT8(0x40, u.bytes[6] & 0xF0);
    /* variant 10xx in the top bits of byte 8 */
    TEST_ASSERT_EQUAL_UINT8(0x80, u.bytes[8] & 0xC0);
}

static void test_distinct_values(void)
{
    d4np_uuid_t a;
    d4np_uuid_t b;
    d4np_uuid_generate(&a);
    d4np_uuid_generate(&b);
    TEST_ASSERT_NOT_EQUAL(0, memcmp(a.bytes, b.bytes, sizeof(a.bytes)));
}

static void test_format_shape(void)
{
    d4np_uuid_t u;
    d4np_uuid_generate(&u);
    char text[D4NP_UUID_STRING_LEN];
    d4np_uuid_format(&u, text);

    TEST_ASSERT_EQUAL_size_t(36, strlen(text));
    /* dashes at the canonical positions */
    TEST_ASSERT_EQUAL_CHAR('-', text[8]);
    TEST_ASSERT_EQUAL_CHAR('-', text[13]);
    TEST_ASSERT_EQUAL_CHAR('-', text[18]);
    TEST_ASSERT_EQUAL_CHAR('-', text[23]);
    TEST_ASSERT_EQUAL_CHAR('4', text[14]); /* version digit */
    /* everything else is a lowercase hex digit */
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            continue;
        }
        char c = text[i];
        bool is_hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        TEST_ASSERT_TRUE(is_hex);
    }
}

void suite_uuid(void)
{
    RUN_TEST(test_version_and_variant_bits);
    RUN_TEST(test_distinct_values);
    RUN_TEST(test_format_shape);
}
