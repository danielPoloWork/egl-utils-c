/*
 * d4np-c — RFC 4122 version-4 UUIDs (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/sys/uuid.h"

#include "d4np/sys/clock.h"

#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER)
#define D4NP_THREAD_LOCAL __declspec(thread)
#else
#define D4NP_THREAD_LOCAL _Thread_local
#endif

/* splitmix64 — a fast, well-distributed PRNG; seeded per thread on first use. */
static uint64_t splitmix64(uint64_t *state)
{
    uint64_t z = (*state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

static D4NP_THREAD_LOCAL uint64_t g_rng_state;
static D4NP_THREAD_LOCAL int g_rng_seeded;

static uint64_t next_random(void)
{
    if (!g_rng_seeded) {
        uint64_t seed = d4np_timestamp_ns();
        seed ^= (uint64_t)(uintptr_t)&g_rng_state; /* per-thread address entropy */
        seed ^= (uint64_t)(uintptr_t)&g_rng_seeded << 17;
        if (seed == 0) {
            seed = 0x123456789ABCDEF0ULL;
        }
        g_rng_state = seed;
        g_rng_seeded = 1;
    }
    return splitmix64(&g_rng_state);
}

void d4np_uuid_generate(d4np_uuid_t *out)
{
    if (out == NULL) {
        return;
    }
    uint64_t a = next_random();
    uint64_t b = next_random();
    memcpy(out->bytes, &a, sizeof(a));
    memcpy(out->bytes + 8, &b, sizeof(b));

    /* RFC 4122: version 4 in the high nibble of byte 6, variant 10x in the top bits of byte 8. */
    out->bytes[6] = (uint8_t)((out->bytes[6] & 0x0F) | 0x40);
    out->bytes[8] = (uint8_t)((out->bytes[8] & 0x3F) | 0x80);
}

void d4np_uuid_format(const d4np_uuid_t *u, char *out)
{
    static const char hex[] = "0123456789abcdef";
    if (u == NULL || out == NULL) {
        return;
    }
    size_t pos = 0;
    for (size_t i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            out[pos++] = '-';
        }
        out[pos++] = hex[(u->bytes[i] >> 4) & 0x0F];
        out[pos++] = hex[u->bytes[i] & 0x0F];
    }
    out[pos] = '\0';
}
