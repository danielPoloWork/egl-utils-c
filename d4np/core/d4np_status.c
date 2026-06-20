/*
 * d4np-c — status code names.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/core/d4np_status.h"

const char *d4np_status_str(d4np_status_t status)
{
    switch (status) {
    case D4NP_OK:
        return "D4NP_OK";
    case D4NP_ERR_INVALID_ARGUMENT:
        return "D4NP_ERR_INVALID_ARGUMENT";
    case D4NP_ERR_OUT_OF_MEMORY:
        return "D4NP_ERR_OUT_OF_MEMORY";
    case D4NP_ERR_OVERFLOW:
        return "D4NP_ERR_OVERFLOW";
    case D4NP_ERR_NOT_FOUND:
        return "D4NP_ERR_NOT_FOUND";
    case D4NP_ERR_IO:
        return "D4NP_ERR_IO";
    case D4NP_ERR_AGAIN:
        return "D4NP_ERR_AGAIN";
    case D4NP_ERR_UNSUPPORTED:
        return "D4NP_ERR_UNSUPPORTED";
    case D4NP_ERR_INTERNAL:
        return "D4NP_ERR_INTERNAL";
    }
    return "D4NP_ERR_UNKNOWN";
}
