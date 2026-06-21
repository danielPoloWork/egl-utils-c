/*
 * d4np-c — tests for whole-file read/write (#18, #19).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

#include <stdio.h>
#include <string.h>

#define TEST_PATH "d4np_io_roundtrip.bin"

static void cleanup(void)
{
    remove(TEST_PATH);
}

static void test_write_then_read_roundtrip(void)
{
    const char *payload = "Daniel;Polo;Architect\nline2\n";
    size_t plen = strlen(payload);

    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_file_write_all(TEST_PATH, payload, plen));

    unsigned char *data = NULL;
    size_t size = 0;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_file_read_all(TEST_PATH, NULL, &data, &size));
    TEST_ASSERT_EQUAL_size_t(plen, size);
    TEST_ASSERT_EQUAL_CHAR_ARRAY(payload, data, plen);
    d4np_free(NULL, data, size);
    cleanup();
}

static void test_atomic_overwrite_replaces_content(void)
{
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_file_write_all(TEST_PATH, "first-version", 13));
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_file_write_all(TEST_PATH, "second", 6)); /* replace */

    unsigned char *data = NULL;
    size_t size = 0;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_file_read_all(TEST_PATH, NULL, &data, &size));
    TEST_ASSERT_EQUAL_size_t(6, size);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("second", data, 6);
    d4np_free(NULL, data, size);
    cleanup();
}

static void test_empty_file_roundtrip(void)
{
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_file_write_all(TEST_PATH, NULL, 0));
    unsigned char *data = (unsigned char *)1; /* sentinel */
    size_t size = 99;
    TEST_ASSERT_EQUAL_INT(D4NP_OK, d4np_file_read_all(TEST_PATH, NULL, &data, &size));
    TEST_ASSERT_NULL(data);
    TEST_ASSERT_EQUAL_size_t(0, size);
    cleanup();
}

static void test_read_missing_file_is_io_error(void)
{
    unsigned char *data = NULL;
    size_t size = 0;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_IO, d4np_file_read_all("d4np_does_not_exist_12345.bin", NULL, &data, &size));
}

static void test_invalid_arguments(void)
{
    unsigned char *data = NULL;
    size_t size = 0;
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_file_read_all(NULL, NULL, &data, &size));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_file_write_all(NULL, "x", 1));
    TEST_ASSERT_EQUAL_INT(D4NP_ERR_INVALID_ARGUMENT, d4np_file_write_all(TEST_PATH, NULL, 5));
}

void suite_file(void)
{
    RUN_TEST(test_write_then_read_roundtrip);
    RUN_TEST(test_atomic_overwrite_replaces_content);
    RUN_TEST(test_empty_file_roundtrip);
    RUN_TEST(test_read_missing_file_is_io_error);
    RUN_TEST(test_invalid_arguments);
}
