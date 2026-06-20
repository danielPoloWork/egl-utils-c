/*
 * d4np-c — unit test runner. Aggregates every module's suite into one Unity executable.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "tests.h"

void setUp(void)
{
}
void tearDown(void)
{
}

int main(void)
{
    UNITY_BEGIN();
    suite_str();
    suite_parse();
    suite_arena();
    suite_pool();
    suite_vector();
    suite_string_builder();
    suite_hashmap();
    suite_linked_list();
    suite_ring_buffer();
    suite_mutex();
    suite_semaphore();
    suite_atomic_queue();
    suite_thread_pool();
    suite_file();
    suite_path();
    suite_hash();
    suite_clock();
    suite_uuid();
    return UNITY_END();
}
