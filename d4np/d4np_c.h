/**
 * @file d4np_c.h
 * @brief Umbrella public header — pulls in the whole d4np-c surface.
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
 *
 * @defgroup d4np_core Core foundation
 * @brief Status codes, pluggable allocator, and the library version.
 *
 * @defgroup d4np_mem Memory & allocators
 * @brief Arena (bump-pointer) and fixed-block pool allocators.
 *
 * @defgroup d4np_ds Data structures
 * @brief Vector, hash map, intrusive linked list, ring buffer, and string builder.
 *
 * @defgroup d4np_concurrency Concurrency & synchronization
 * @brief Portable mutex, counting semaphore, lock-free SPSC queue, and thread pool.
 *
 * @defgroup d4np_str Strings & parsing
 * @brief Non-owning string views, splitting, and overflow-safe number parsing.
 *
 * @defgroup d4np_io File system & I/O
 * @brief Whole-file read/write and OS-aware path joining.
 *
 * @defgroup d4np_sys System utilities & diagnostics
 * @brief Logger, thread-local error context, monotonic clock, UUIDv4, and FNV-1a hashing.
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

/* Concurrency & synchronization */
#include "d4np/concurrency/atomic_queue.h"
#include "d4np/concurrency/mutex.h"
#include "d4np/concurrency/named_semaphore.h"
#include "d4np/concurrency/semaphore.h"
#include "d4np/concurrency/thread_pool.h"

/* Generic data structures */
#include "d4np/ds/hashmap.h"
#include "d4np/ds/linked_list.h"
#include "d4np/ds/ring_buffer.h"
#include "d4np/ds/string_builder.h"
#include "d4np/ds/vector.h"

/* File system & I/O */
#include "d4np/io/file.h"
#include "d4np/io/path.h"

/* System utilities & diagnostics */
#include "d4np/sys/clock.h"
#include "d4np/sys/error_context.h"
#include "d4np/sys/hash.h"
#include "d4np/sys/log.h"
#include "d4np/sys/uuid.h"

/* Strings & parsing */
#include "d4np/str/parse.h"
#include "d4np/str/str_view.h"

#endif /* D4NP_C_H */
