/*
 * d4np-c — monotonic clock (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
/* clock_gettime / CLOCK_MONOTONIC need POSIX.1; the build compiles as strict -std=c11. */
#define _POSIX_C_SOURCE 200809L

#include "d4np/sys/clock.h"

#if defined(_WIN32)
#include <windows.h>

uint64_t d4np_timestamp_ns(void)
{
    LARGE_INTEGER freq;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    uint64_t f = (uint64_t)freq.QuadPart;
    uint64_t t = (uint64_t)counter.QuadPart;
    /* Split to avoid overflow in t * 1e9. */
    return (t / f) * 1000000000ULL + ((t % f) * 1000000000ULL) / f;
}
#else
#include <time.h>

uint64_t d4np_timestamp_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}
#endif

uint64_t d4np_timestamp_ms(void)
{
    return d4np_timestamp_ns() / 1000000ULL;
}
