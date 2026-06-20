/*
 * d4np-c — whole-file read/write helpers (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
/* fileno() / fsync() are POSIX, not ISO C; the build compiles as strict -std=c11. */
#define _POSIX_C_SOURCE 200809L

#include "d4np/io/file.h"

#include <stdalign.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

enum { D4NP_FILE_CHUNK = 65536 };

typedef union d4np_file_max_align {
    long double ld;
    long long ll;
    void *p;
    void (*fp)(void);
} d4np_file_max_align_t;

d4np_status_t d4np_file_read_all(const char *path, const d4np_allocator_t *allocator, unsigned char **out_data,
                                 size_t *out_size)
{
    if (path == NULL || out_data == NULL || out_size == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (allocator == NULL) {
        allocator = d4np_allocator_default();
    }
    const size_t align = alignof(d4np_file_max_align_t);

    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        return D4NP_ERR_IO;
    }

    unsigned char *buf = NULL;
    size_t cap = 0;
    size_t len = 0;
    for (;;) {
        if (len == cap) {
            size_t ncap = (cap == 0) ? (size_t)D4NP_FILE_CHUNK : cap * 2;
            unsigned char *nb = (unsigned char *)d4np_realloc(allocator, buf, cap, ncap, align);
            if (nb == NULL) {
                d4np_free(allocator, buf, cap);
                fclose(f);
                return D4NP_ERR_OUT_OF_MEMORY;
            }
            buf = nb;
            cap = ncap;
        }
        size_t got = fread(buf + len, 1, cap - len, f);
        len += got;
        if (got == 0) {
            if (ferror(f)) {
                d4np_free(allocator, buf, cap);
                fclose(f);
                return D4NP_ERR_IO;
            }
            break; /* EOF */
        }
    }
    fclose(f);

    if (len == 0) {
        d4np_free(allocator, buf, cap);
        *out_data = NULL;
        *out_size = 0;
        return D4NP_OK;
    }
    /* Trim the over-allocation so the caller's free size matches *out_size. */
    unsigned char *shrunk = (unsigned char *)d4np_realloc(allocator, buf, cap, len, align);
    if (shrunk != NULL) {
        buf = shrunk;
    }
    *out_data = buf;
    *out_size = len;
    return D4NP_OK;
}

static int file_sync(FILE *f)
{
#if defined(_WIN32)
    return _commit(_fileno(f));
#else
    return fsync(fileno(f));
#endif
}

d4np_status_t d4np_file_write_all(const char *path, const void *data, size_t size)
{
    if (path == NULL || (size > 0 && data == NULL)) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }

    char tmp[4096];
    size_t plen = strlen(path);
    if (plen + sizeof(".tmp") > sizeof(tmp)) {
        return D4NP_ERR_INVALID_ARGUMENT; /* path too long for the temp-name buffer */
    }
    memcpy(tmp, path, plen);
    memcpy(tmp + plen, ".tmp", sizeof(".tmp")); /* includes NUL */

    FILE *f = fopen(tmp, "wb");
    if (f == NULL) {
        return D4NP_ERR_IO;
    }
    if (size > 0 && fwrite(data, 1, size, f) != size) {
        fclose(f);
        remove(tmp);
        return D4NP_ERR_IO;
    }
    if (fflush(f) != 0 || file_sync(f) != 0) {
        fclose(f);
        remove(tmp);
        return D4NP_ERR_IO;
    }
    if (fclose(f) != 0) {
        remove(tmp);
        return D4NP_ERR_IO;
    }

    /* Atomic replace of the target. */
#if defined(_WIN32)
    if (!MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING)) {
        remove(tmp);
        return D4NP_ERR_IO;
    }
#else
    if (rename(tmp, path) != 0) {
        remove(tmp);
        return D4NP_ERR_IO;
    }
#endif
    return D4NP_OK;
}
