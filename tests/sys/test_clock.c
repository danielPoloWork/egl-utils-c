/*
 * d4np-c — tests for the monotonic clock (#23).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

static void test_ns_is_monotonic(void)
{
    uint64_t a = d4np_timestamp_ns();
    uint64_t b = d4np_timestamp_ns();
    TEST_ASSERT_TRUE(b >= a); /* never goes backwards */
}

static void test_ms_is_monotonic_and_consistent_with_ns(void)
{
    uint64_t ns = d4np_timestamp_ns();
    uint64_t ms = d4np_timestamp_ms();
    uint64_t ns2 = d4np_timestamp_ns();
    /* ms reading was taken between the two ns readings, so ms*1e6 sits within [ns, ns2] modulo
     * truncation; assert it is in a sane neighbourhood rather than exact equality. */
    TEST_ASSERT_TRUE(ms * 1000000ULL <= ns2 + 2000000ULL);
    TEST_ASSERT_TRUE(ms * 1000000ULL + 2000000ULL >= ns);
}

static void test_elapsed_is_nonnegative_over_work(void)
{
    uint64_t start = d4np_timestamp_ns();
    volatile uint64_t acc = 0;
    for (int i = 0; i < 100000; ++i) {
        acc += (uint64_t)i;
    }
    uint64_t end = d4np_timestamp_ns();
    TEST_ASSERT_TRUE(end >= start);
    (void)acc;
}

void suite_clock(void)
{
    RUN_TEST(test_ns_is_monotonic);
    RUN_TEST(test_ms_is_monotonic_and_consistent_with_ns);
    RUN_TEST(test_elapsed_is_nonnegative_over_work);
}
