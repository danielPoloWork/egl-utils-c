/*
 * d4np-c — leveled logger (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * The logger lazily initializes its mutex and config on first use via an atomic one-time guard,
 * so callers need no explicit init. The output stream is read/written under the mutex; the
 * minimum level is a relaxed atomic for a cheap, lock-free level check on the drop path.
 */
#include "d4np/sys/log.h"

#include "d4np/concurrency/mutex.h"
#include "d4np/sys/clock.h"

#include <stdarg.h>
#include <stdatomic.h>

static atomic_int g_once;      /* 0 = uninitialized, 1 = initializing, 2 = ready */
static atomic_int g_min_level; /* d4np_log_level_t as int */
static d4np_mutex_t g_mutex;
static FILE *g_stream;

static void log_ensure_init(void)
{
    if (atomic_load_explicit(&g_once, memory_order_acquire) == 2) {
        return;
    }
    int expected = 0;
    if (atomic_compare_exchange_strong(&g_once, &expected, 1)) {
        d4np_mutex_init(&g_mutex);
        g_stream = stderr;
        atomic_store_explicit(&g_min_level, (int)D4NP_LOG_INFO, memory_order_relaxed);
        atomic_store_explicit(&g_once, 2, memory_order_release);
    } else {
        while (atomic_load_explicit(&g_once, memory_order_acquire) != 2) {
            /* another thread is initializing; spin briefly */
        }
    }
}

static const char *level_name(d4np_log_level_t level)
{
    switch (level) {
    case D4NP_LOG_INFO:
        return "INFO";
    case D4NP_LOG_WARN:
        return "WARN";
    case D4NP_LOG_ERROR:
        return "ERROR";
    }
    return "?";
}

void d4np_log_set_output(FILE *stream)
{
    log_ensure_init();
    d4np_mutex_lock(&g_mutex);
    g_stream = (stream != NULL) ? stream : stderr;
    d4np_mutex_unlock(&g_mutex);
}

void d4np_log_set_min_level(d4np_log_level_t level)
{
    log_ensure_init();
    atomic_store_explicit(&g_min_level, (int)level, memory_order_relaxed);
}

void d4np_log_write(d4np_log_level_t level, const char *fmt, ...)
{
    log_ensure_init();
    if ((int)level < atomic_load_explicit(&g_min_level, memory_order_relaxed)) {
        return;
    }

    unsigned long long ts = (unsigned long long)d4np_timestamp_ms();
    va_list ap;
    va_start(ap, fmt);

    d4np_mutex_lock(&g_mutex);
    fprintf(g_stream, "[%llu] %-5s ", ts, level_name(level));
    vfprintf(g_stream, fmt, ap);
    fputc('\n', g_stream);
    fflush(g_stream);
    d4np_mutex_unlock(&g_mutex);

    va_end(ap);
}
