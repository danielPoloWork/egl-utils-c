/*
 * d4np-c — test suite registry.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * One executable (d4np_tests) aggregates every module's tests. Each module test file keeps its
 * test functions static and exposes a single `suite_<module>()` that issues the RUN_TEST calls;
 * tests/test_main.c owns setUp/tearDown and the UNITY_BEGIN/END bracket and calls each suite.
 */
#ifndef D4NP_TESTS_H
#define D4NP_TESTS_H

void suite_str(void);
void suite_parse(void);
void suite_arena(void);
void suite_pool(void);
void suite_vector(void);
void suite_string_builder(void);
void suite_hashmap(void);
void suite_linked_list(void);
void suite_ring_buffer(void);
void suite_mutex(void);
void suite_semaphore(void);
void suite_atomic_queue(void);
void suite_thread_pool(void);
void suite_file(void);
void suite_path(void);

#endif /* D4NP_TESTS_H */
