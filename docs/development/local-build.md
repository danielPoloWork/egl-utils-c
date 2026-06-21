# Local Build & Test

How to build, test, and check `d4np-c` on your machine. CI runs the same commands
on Linux x86_64 (GCC>=11, Clang>=14), Windows x86_64 (MSVC>=19.30), macOS arm64 (Apple Clang>=14); reproducing them locally avoids a red round-trip.

## Prerequisites

- **C11** toolchain.
- **Build system:** CMake + Ninja.
- **Package manager:** vcpkg / Conan (test deps only; library stays dependency-free).
- **Formatter / linter:** clang-format, clang-tidy + cppcheck.
- **Docs:** Doxygen (for the API docs build).

## Commands

```bash
# Build
cmake --build --preset debug

# Test
ctest --preset debug --output-on-failure

# Format check
clang-format --dry-run --Werror <files>

# Lint
clang-tidy --warnings-as-errors='*' <changed .c/.h>

# Benchmark
cmake --build --preset bench

# Line coverage (GCC/Clang) — builds + tests instrumented, enforces the >=80% gate over d4np/
tools/coverage.sh            # optionally: tools/coverage.sh <threshold>
#   Clang: GCOV="llvm-cov gcov" tools/coverage.sh

# Cross-artifact congruence (run before drafting any PR)
python tools/consistency_lint.py
```

## Before you open a PR

1. `clang-format --dry-run --Werror <files>` and `clang-tidy --warnings-as-errors='*' <changed .c/.h>` are clean.
2. `ctest --preset debug --output-on-failure` passes; new/changed behavior is covered (≥ 80% line).
3. ASan, UBSan, TSan, Valgrind are green where applicable.
4. `python tools/consistency_lint.py` passes.
5. The relevant docs (README, ROADMAP, ADRs, patterns, changelog) are updated in the same
   PR — see [`../workflow/documentation.md`](../workflow/documentation.md).
