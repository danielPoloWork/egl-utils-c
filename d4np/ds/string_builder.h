/**
 * @file string_builder.h
 * @brief Dynamic string builder.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Efficient incremental assembly of a string with amortized O(1) appends (geometric growth)
 * and no quadratic re-copying. The buffer is always kept NUL-terminated so d4np_sb_cstr() is
 * O(1) and safe to hand to C string APIs. Allocator-injected; owns its buffer (spec \#10).
 *
 * @ingroup d4np_ds
 */
#ifndef D4NP_DS_STRING_BUILDER_H
#define D4NP_DS_STRING_BUILDER_H

#include <stddef.h>

#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"
#include "d4np/str/str_view.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_ds
 * @{
 */

/** @brief A geometrically growing, always-NUL-terminated string buffer. */
typedef struct d4np_string_builder {
    const d4np_allocator_t *allocator; /**< backing allocator the buffer came from */
    char *data;                        /**< NUL-terminated buffer, or NULL before first allocation */
    size_t len;                        /**< bytes used, excluding the terminating NUL              */
    size_t capacity;                   /**< bytes allocated, including room for the NUL            */
} d4np_string_builder_t;

/**
 * @brief Initialize a string builder.
 * @param sb               Builder to initialize.
 * @param allocator        Backing allocator, or NULL to select the default allocator.
 * @param initial_capacity Capacity hint in bytes; 0 means lazy allocation.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INVALID_ARGUMENT or ::D4NP_ERR_OUT_OF_MEMORY on
 *         failure.
 */
d4np_status_t d4np_sb_init(d4np_string_builder_t *sb, const d4np_allocator_t *allocator, size_t initial_capacity);

/**
 * @brief Release the buffer and zero the builder.
 * @param sb Builder to destroy.
 * @note Safe on a zeroed builder.
 */
void d4np_sb_destroy(d4np_string_builder_t *sb);

/**
 * @brief Ensure the buffer can hold at least @p min_capacity bytes (including the NUL).
 * @param sb           Builder to grow.
 * @param min_capacity Minimum capacity in bytes, including the terminating NUL.
 * @return ::D4NP_OK on success, or an error status on failure.
 */
d4np_status_t d4np_sb_reserve(d4np_string_builder_t *sb, size_t min_capacity);

/**
 * @brief Append a NUL-terminated C string.
 * @param sb   Builder to append to.
 * @param cstr NUL-terminated string to append.
 * @return ::D4NP_OK on success, or an error status on failure.
 */
d4np_status_t d4np_sb_append(d4np_string_builder_t *sb, const char *cstr);

/**
 * @brief Append the bytes of a string view (no NUL assumptions).
 * @param sb   Builder to append to.
 * @param view View whose bytes are appended.
 * @return ::D4NP_OK on success, or an error status on failure.
 */
d4np_status_t d4np_sb_append_view(d4np_string_builder_t *sb, d4np_str_view_t view);

/**
 * @brief Append a single byte.
 * @param sb Builder to append to.
 * @param c  Byte to append.
 * @return ::D4NP_OK on success, or an error status on failure.
 */
d4np_status_t d4np_sb_append_char(d4np_string_builder_t *sb, char c);

/**
 * @brief NUL-terminated contents.
 * @param sb Builder to read.
 * @return The contents, or "" for an empty/zeroed builder.
 * @note The returned pointer is valid until the next mutation.
 */
const char *d4np_sb_cstr(const d4np_string_builder_t *sb);

/**
 * @brief Non-owning view over the contents (no copy).
 * @param sb Builder to read.
 * @return A view over the current contents.
 * @note The returned view is valid until the next mutation.
 */
d4np_str_view_t d4np_sb_view(const d4np_string_builder_t *sb);

/** @brief Number of bytes used, excluding the terminating NUL. */
size_t d4np_sb_len(const d4np_string_builder_t *sb);

/**
 * @brief Reset to empty, keeping the allocated buffer.
 * @param sb Builder to clear.
 */
void d4np_sb_clear(d4np_string_builder_t *sb);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_DS_STRING_BUILDER_H */
