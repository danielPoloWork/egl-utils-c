/*
 * d4np-c — robust string-to-number parsing.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Overflow-safe conversions from a (non-owning) string view to numbers, reporting failure
 * through d4np_status_t rather than errno or sentinel returns. Parsing operates on the view's
 * bytes directly (no NUL required); leading and trailing ASCII whitespace is ignored, but any
 * other trailing character makes the whole parse fail — there is no "parse a prefix" mode
 * (spec #17).
 */
#ifndef D4NP_STR_PARSE_H
#define D4NP_STR_PARSE_H

#include <stdint.h>

#include "d4np/core/d4np_status.h"
#include "d4np/str/str_view.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Parse a signed integer from `sv` into *out. `base` is 2..36, or 0 to auto-detect from the
 * prefix (0x -> 16, leading 0 -> 8, else 10). An optional leading '+'/'-' is accepted; a "0x"
 * prefix is also accepted when base is 16. Returns:
 *   - D4NP_OK             on a fully-consumed valid number;
 *   - D4NP_ERR_INVALID_ARGUMENT for NULL out, a NULL/empty view, no digits, a bad base, or
 *     trailing non-whitespace;
 *   - D4NP_ERR_OVERFLOW   if the value does not fit in int64_t.
 */
d4np_status_t d4np_str_parse_int(d4np_str_view_t sv, int base, int64_t *out);

/*
 * Parse a double from `sv` into *out (decimal or hex float, with optional exponent, inf/nan, as
 * accepted by strtod). Returns D4NP_OK; D4NP_ERR_INVALID_ARGUMENT for NULL out, a NULL/empty
 * view, trailing non-whitespace, or a token longer than the internal scratch buffer; or
 * D4NP_ERR_OVERFLOW on magnitude overflow (HUGE_VAL).
 */
d4np_status_t d4np_str_parse_float(d4np_str_view_t sv, double *out);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_STR_PARSE_H */
