/*
 * d4np-c — tests for FNV-1a hashing (#25).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

#include <string.h>

static void test_known_vectors(void)
{
    /* Canonical FNV-1a 64-bit test vectors. */
    TEST_ASSERT_EQUAL_HEX64(0xcbf29ce484222325ULL, d4np_hash_fnv1a_str(""));
    TEST_ASSERT_EQUAL_HEX64(0xaf63dc4c8601ec8cULL, d4np_hash_fnv1a_str("a"));
    TEST_ASSERT_EQUAL_HEX64(0x85944171f73967e8ULL, d4np_hash_fnv1a_str("foobar"));
}

static void test_null_hashes_to_basis(void)
{
    TEST_ASSERT_EQUAL_HEX64(D4NP_FNV1A_OFFSET_BASIS, d4np_hash_fnv1a(NULL, 0));
    TEST_ASSERT_EQUAL_HEX64(D4NP_FNV1A_OFFSET_BASIS, d4np_hash_fnv1a("", 0));
}

static void test_continuation_matches_one_shot(void)
{
    uint64_t chained = d4np_hash_fnv1a_continue(D4NP_FNV1A_OFFSET_BASIS, "foo", 3);
    chained = d4np_hash_fnv1a_continue(chained, "bar", 3);
    TEST_ASSERT_EQUAL_HEX64(d4np_hash_fnv1a_str("foobar"), chained);
}

static void test_binary_data(void)
{
    unsigned char a[] = {0x00, 0x01, 0x02, 0xFF};
    unsigned char b[] = {0x00, 0x01, 0x02, 0xFE};
    TEST_ASSERT_NOT_EQUAL(d4np_hash_fnv1a(a, sizeof(a)), d4np_hash_fnv1a(b, sizeof(b)));
}

void suite_hash(void)
{
    RUN_TEST(test_known_vectors);
    RUN_TEST(test_null_hashes_to_basis);
    RUN_TEST(test_continuation_matches_one_shot);
    RUN_TEST(test_binary_data);
}
