/*
 * d4np-c — thread-local error context (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/sys/error_context.h"

#if defined(_MSC_VER)
#define D4NP_THREAD_LOCAL __declspec(thread)
#else
#define D4NP_THREAD_LOCAL _Thread_local
#endif

#define D4NP_ERROR_MESSAGE_CAP 256

typedef struct error_frame {
    d4np_status_t status;
    char message[D4NP_ERROR_MESSAGE_CAP];
} error_frame;

static D4NP_THREAD_LOCAL error_frame g_stack[D4NP_ERROR_CONTEXT_MAX_DEPTH];
static D4NP_THREAD_LOCAL size_t g_depth;

d4np_status_t d4np_error_context_push(d4np_status_t status, const char *message)
{
    if (g_depth < D4NP_ERROR_CONTEXT_MAX_DEPTH) {
        error_frame *f = &g_stack[g_depth++];
        f->status = status;
        size_t i = 0;
        if (message != NULL) {
            for (; i + 1 < D4NP_ERROR_MESSAGE_CAP && message[i] != '\0'; ++i) {
                f->message[i] = message[i];
            }
        }
        f->message[i] = '\0';
    }
    return status;
}

bool d4np_error_context_pop(d4np_status_t *out_status, const char **out_message)
{
    if (g_depth == 0) {
        return false;
    }
    error_frame *f = &g_stack[--g_depth];
    if (out_status != NULL) {
        *out_status = f->status;
    }
    if (out_message != NULL) {
        *out_message = f->message;
    }
    return true;
}

void d4np_error_context_clear(void)
{
    g_depth = 0;
}

size_t d4np_error_context_depth(void)
{
    return g_depth;
}
