/*
 * d4np-c — monotonic clock.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A steady, monotonic timer for measuring elapsed time and stamping logs (spec #23). It never
 * goes backwards and is unaffected by wall-clock changes; the epoch is unspecified, so only
 * differences between two readings are meaningful.
 */
#ifndef D4NP_SYS_CLOCK_H
#define D4NP_SYS_CLOCK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Monotonic milliseconds since an unspecified epoch. */
uint64_t d4np_timestamp_ms(void);

/* Monotonic nanoseconds since an unspecified epoch (higher resolution). */
uint64_t d4np_timestamp_ns(void);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_SYS_CLOCK_H */
