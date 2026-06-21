/*
 * d4np-c — dynamic string builder.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Efficient incremental assembly of a string with amortized O(1) appends (geometric growth)
 * and no quadratic re-copying. The buffer is always kept NUL-terminated so d4np_sb_cstr() is
 * O(1) and safe to hand to C string APIs. Allocator-injected; owns its buffer (spec #10).
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

typedef struct d4np_string_builder {
    const d4np_allocator_t *allocator;
    char *data;      /* NUL-terminated buffer, or NULL before first allocation */
    size_t len;      /* bytes used, excluding the terminating NUL              */
    size_t capacity; /* bytes allocated, including room for the NUL            */
} d4np_string_builder_t;

/*
 * Initialize `sb`, drawn from `allocator` (NULL -> default). `initial_capacity` is a hint in
 * bytes (0 = lazy). Returns D4NP_OK, D4NP_ERR_INVALID_ARGUMENT, or D4NP_ERR_OUT_OF_MEMORY.
 */
d4np_status_t d4np_sb_init(d4np_string_builder_t *sb, const d4np_allocator_t *allocator, size_t initial_capacity);

/* Release the buffer and zero the builder. Safe on a zeroed builder. */
void d4np_sb_destroy(d4np_string_builder_t *sb);

/* Ensure the buffer can hold at least `min_capacity` bytes (including the NUL). */
d4np_status_t d4np_sb_reserve(d4np_string_builder_t *sb, size_t min_capacity);

/* Append a NUL-terminated C string. */
d4np_status_t d4np_sb_append(d4np_string_builder_t *sb, const char *cstr);

/* Append the bytes of a string view (no NUL assumptions). */
d4np_status_t d4np_sb_append_view(d4np_string_builder_t *sb, d4np_str_view_t view);

/* Append a single byte. */
d4np_status_t d4np_sb_append_char(d4np_string_builder_t *sb, char c);

/* NUL-terminated contents; returns "" for an empty/zeroed builder. Valid until next mutation. */
const char *d4np_sb_cstr(const d4np_string_builder_t *sb);

/* Non-owning view over the contents (no copy). Valid until next mutation. */
d4np_str_view_t d4np_sb_view(const d4np_string_builder_t *sb);

size_t d4np_sb_len(const d4np_string_builder_t *sb);

/* Reset to empty, keeping the allocated buffer. */
void d4np_sb_clear(d4np_string_builder_t *sb);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_DS_STRING_BUILDER_H */
