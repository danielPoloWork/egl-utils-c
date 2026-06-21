/*
 * d4np-c — leveled logger.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A small, thread-safe, printf-style logger with severity levels and a configurable sink
 * (default stderr) (spec #21). Each record is prefixed with a monotonic timestamp and the
 * level; records below the configured minimum level are dropped cheaply. Writes are serialized
 * by an internal mutex, so lines from concurrent threads never interleave.
 */
#ifndef D4NP_SYS_LOG_H
#define D4NP_SYS_LOG_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum d4np_log_level { D4NP_LOG_INFO = 0, D4NP_LOG_WARN = 1, D4NP_LOG_ERROR = 2 } d4np_log_level_t;

/* Direct output to `stream` (NULL resets to stderr). Thread-safe. */
void d4np_log_set_output(FILE *stream);

/* Drop records strictly below `level`. Default: D4NP_LOG_INFO (log everything). Thread-safe. */
void d4np_log_set_min_level(d4np_log_level_t level);

/* Write one printf-style record at `level` (newline appended). Thread-safe. */
void d4np_log_write(d4np_log_level_t level, const char *fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;

#ifdef __cplusplus
}
#endif

#endif /* D4NP_SYS_LOG_H */
