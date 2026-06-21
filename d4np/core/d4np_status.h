/**
 * @file d4np_status.h
 * @brief Deterministic status codes shared across d4np-c.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Every fallible function in d4np-c returns a constant ::d4np_status_t. The zero value is
 * always success, so callers may write `if (rc != D4NP_OK)`. Per-thread error context
 * (messages / trace) lives in the sys module; this header is the bare code enum that the
 * whole library shares.
 *
 * @ingroup d4np_core
 */
#ifndef D4NP_CORE_STATUS_H
#define D4NP_CORE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_core
 * @{
 */

/** @brief Library-wide status codes; ::D4NP_OK is always 0 (success). */
typedef enum d4np_status {
    D4NP_OK = 0,               /**< success                                            */
    D4NP_ERR_INVALID_ARGUMENT, /**< a NULL or out-of-range argument was passed         */
    D4NP_ERR_OUT_OF_MEMORY,    /**< an allocator returned no memory                    */
    D4NP_ERR_OVERFLOW,         /**< a size/numeric computation overflowed              */
    D4NP_ERR_NOT_FOUND,        /**< a lookup found no matching element                 */
    D4NP_ERR_IO,               /**< an underlying I/O operation failed                 */
    D4NP_ERR_AGAIN,            /**< non-blocking operation would block; retry          */
    D4NP_ERR_UNSUPPORTED,      /**< operation not supported on this platform/build     */
    D4NP_ERR_INTERNAL          /**< an invariant was violated (a bug)                  */
} d4np_status_t;

/**
 * @brief Return a stable, human-readable identifier for a status code.
 *
 * @param status Status code to describe (e.g. ::D4NP_OK yields "D4NP_OK").
 * @return Pointer to a static string that must not be freed. Unknown values yield
 *         "D4NP_ERR_UNKNOWN".
 */
const char *d4np_status_str(d4np_status_t status);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CORE_STATUS_H */
