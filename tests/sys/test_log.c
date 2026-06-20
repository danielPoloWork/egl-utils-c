/*
 * d4np-c — tests for the leveled logger (#21).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

#include <stdio.h>
#include <string.h>

/* Capture log output into a temp file, then read it back for assertions. */
static size_t read_back(FILE *f, char *buf, size_t cap)
{
    fflush(f);
    rewind(f);
    size_t n = fread(buf, 1, cap - 1, f);
    buf[n] = '\0';
    return n;
}

static void test_writes_level_and_message(void)
{
    FILE *f = tmpfile();
    TEST_ASSERT_NOT_NULL(f);
    d4np_log_set_output(f);
    d4np_log_set_min_level(D4NP_LOG_INFO);

    d4np_log_write(D4NP_LOG_ERROR, "boom %s %d", "x", 7);

    char buf[512];
    read_back(f, buf, sizeof(buf));
    TEST_ASSERT_NOT_NULL(strstr(buf, "ERROR"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "boom x 7"));

    d4np_log_set_output(NULL); /* restore stderr before closing */
    fclose(f);
}

static void test_min_level_filters(void)
{
    FILE *f = tmpfile();
    TEST_ASSERT_NOT_NULL(f);
    d4np_log_set_output(f);
    d4np_log_set_min_level(D4NP_LOG_WARN);

    d4np_log_write(D4NP_LOG_INFO, "hidden-info");  /* below threshold: dropped */
    d4np_log_write(D4NP_LOG_WARN, "shown-warn");   /* at threshold: kept */
    d4np_log_write(D4NP_LOG_ERROR, "shown-error"); /* above threshold: kept */

    char buf[512];
    read_back(f, buf, sizeof(buf));
    TEST_ASSERT_NULL(strstr(buf, "hidden-info"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "shown-warn"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "shown-error"));

    d4np_log_set_min_level(D4NP_LOG_INFO); /* restore defaults */
    d4np_log_set_output(NULL);
    fclose(f);
}

void suite_log(void)
{
    RUN_TEST(test_writes_level_and_message);
    RUN_TEST(test_min_level_filters);
}
