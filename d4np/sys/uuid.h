/*
 * d4np-c — RFC 4122 version-4 UUIDs.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Fast generation of random (version 4) UUIDs with the correct version and variant bits
 * (spec #24). The generator is a fast per-thread PRNG, NOT a cryptographic RNG: the UUIDs are
 * unique with overwhelming probability but must not be used as secrets or unguessable tokens.
 */
#ifndef D4NP_SYS_UUID_H
#define D4NP_SYS_UUID_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 36 hex/dash characters + a NUL terminator. */
#define D4NP_UUID_STRING_LEN 37

typedef struct d4np_uuid {
    uint8_t bytes[16];
} d4np_uuid_t;

/* Fill *out with a new version-4 UUID. */
void d4np_uuid_generate(d4np_uuid_t *out);

/* Write the canonical "8-4-4-4-12" lowercase form into `out` (must hold D4NP_UUID_STRING_LEN). */
void d4np_uuid_format(const d4np_uuid_t *u, char *out);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_SYS_UUID_H */
