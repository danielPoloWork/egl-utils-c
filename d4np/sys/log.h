/**
 * @file log.h
 * @brief Leveled logger.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A small, thread-safe, printf-style logger with severity levels and a configurable sink
 * (default stderr) (spec \#21). Each record is prefixed with a monotonic timestamp and the
 * level; records below the configured minimum level are dropped cheaply. Writes are serialized
 * by an internal mutex, so lines from concurrent threads never interleave.
 *
 * @ingroup d4np_sys
 */
#ifndef D4NP_SYS_LOG_H
#define D4NP_SYS_LOG_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_sys
 * @{
 */

/** @brief Severity levels for log records, ordered from least to most severe. */
typedef enum d4np_log_level {
    D4NP_LOG_INFO = 0, /**< informational message */
    D4NP_LOG_WARN = 1, /**< warning message */
    D4NP_LOG_ERROR = 2 /**< error message */
} d4np_log_level_t;

/**
 * @brief Direct output to @p stream.
 * @param stream Destination stream; NULL resets the sink to stderr.
 * @note Thread-safe.
 */
void d4np_log_set_output(FILE *stream);

/**
 * @brief Drop records strictly below @p level.
 * @param level Minimum severity to emit. Default: ::D4NP_LOG_INFO (log everything).
 * @note Thread-safe.
 */
void d4np_log_set_min_level(d4np_log_level_t level);

/**
 * @brief Write one printf-style record at @p level (newline appended).
 * @param level Severity of the record.
 * @param fmt   printf-style format string.
 * @param ...   Arguments consumed by @p fmt.
 * @note Thread-safe.
 */
void d4np_log_write(d4np_log_level_t level, const char *fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_SYS_LOG_H */
