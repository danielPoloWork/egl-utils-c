/**
 * @file uuid.h
 * @brief RFC 4122 version-4 UUIDs.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Fast generation of random (version 4) UUIDs with the correct version and variant bits
 * (spec \#24). The generator is a fast per-thread PRNG, NOT a cryptographic RNG: the UUIDs are
 * unique with overwhelming probability but must not be used as secrets or unguessable tokens.
 *
 * @ingroup d4np_sys
 */
#ifndef D4NP_SYS_UUID_H
#define D4NP_SYS_UUID_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_sys
 * @{
 */

/** @brief Length of the canonical UUID string: 36 hex/dash characters + a NUL terminator. */
#define D4NP_UUID_STRING_LEN 37

/** @brief A 128-bit UUID held as 16 raw bytes. */
typedef struct d4np_uuid {
    uint8_t bytes[16]; /**< the 16 raw bytes of the UUID */
} d4np_uuid_t;

/**
 * @brief Fill @p out with a new version-4 UUID.
 * @param out Destination UUID (must be non-NULL).
 * @warning Generated from a fast per-thread PRNG, NOT a cryptographic RNG: unique with
 *          overwhelming probability but unsuitable for secrets or unguessable tokens.
 */
void d4np_uuid_generate(d4np_uuid_t *out);

/**
 * @brief Write the canonical "8-4-4-4-12" lowercase form into @p out.
 * @param u   UUID to format.
 * @param out Destination buffer; must hold at least ::D4NP_UUID_STRING_LEN bytes.
 */
void d4np_uuid_format(const d4np_uuid_t *u, char *out);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_SYS_UUID_H */
