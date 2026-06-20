/*
 * d4np-c — umbrella public header.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Include this single header to pull in the whole public surface:
 *
 *     #include "d4np_c.h"
 *
 * Or include a single module's public header directly, e.g.:
 *
 *     #include "d4np/str/str_view.h"
 *
 * As modules from the spec land, their public headers are added below.
 */
#ifndef D4NP_C_H
#define D4NP_C_H

/* Foundation */
#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"
#include "d4np/core/version.h"

/* Memory & allocators */
#include "d4np/mem/arena.h"
#include "d4np/mem/pool.h"

/* Generic data structures */
#include "d4np/ds/string_builder.h"
#include "d4np/ds/vector.h"

/* Strings & parsing */
#include "d4np/str/str_view.h"

#endif /* D4NP_C_H */
