#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Daniel Polo
#
# Reproducible line-coverage gate for d4np-c (roadmap M8.1; see ADR-0003).
#
# Configures, builds, and runs the unit tests under the `coverage` CMake preset
# (GCC/Clang `--coverage` instrumentation), then drives gcovr over the d4np/
# module sources and FAILS when line coverage drops below the threshold.
#
# Usage:
#   tools/coverage.sh [THRESHOLD]      # THRESHOLD defaults to 80 (percent)
#
# Environment:
#   GCOV   gcov front-end to use (default: gcov). For a Clang build, export
#          GCOV="llvm-cov gcov" so gcovr parses LLVM-format counters.
#
# Outputs (under ./coverage/, git-ignored):
#   coverage.xml          Cobertura XML (for CI ingestion / badges)
#   html/index.html       browsable per-file report
set -euo pipefail

THRESHOLD="${1:-80}"
GCOV="${GCOV:-gcov}"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if ! command -v gcovr >/dev/null 2>&1; then
    echo "error: gcovr is not installed (pip install gcovr)" >&2
    exit 127
fi

echo "==> Configuring + building the coverage preset"
cmake --preset coverage
cmake --build --preset coverage

echo "==> Running the unit tests (emits .gcda counters)"
ctest --preset coverage --output-on-failure

echo "==> Aggregating coverage over d4np/ (threshold: ${THRESHOLD}% line)"
mkdir -p coverage/html
gcovr \
    --root "$ROOT" \
    --gcov-executable "$GCOV" \
    --filter 'd4np/' \
    --exclude-unreachable-branches \
    --print-summary \
    --xml coverage/coverage.xml --xml-pretty \
    --html-details coverage/html/index.html \
    --fail-under-line "$THRESHOLD"
