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
    suite_arena();
    suite_pool();
    return UNITY_END();
}
