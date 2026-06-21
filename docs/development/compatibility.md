# Compatibility Matrix

The supported language standard, compilers, and platforms for `d4np-c`, and the broad-reach
build that keeps them honest. See [ADR-0006](../adr/0006-require-c11-language-floor.md) for the
rationale.

## Language standard: C11 is the floor

`d4np-c` targets **ISO/IEC 9899:2011 (C11)** and does **not** support C99 or earlier. This is a
hard floor, not a preference: the library's core value (lock-free concurrency) and its ergonomics
depend on features that have no portable pre-C11 equivalent.

| C11 feature | Why it is required | Where it is used |
|-------------|--------------------|------------------|
| `<stdatomic.h>` / `_Atomic` | Lock-free SPSC structures need standard atomics with explicit memory ordering — there is no portable C99 equivalent | `concurrency/atomic_queue`, `ds/ring_buffer`, `sys/log` |
| `<stdalign.h>` / `alignof` | Honoring caller-requested and natural alignment in the allocators and containers | `core/allocator`, `mem/arena`, `mem/pool`, `ds/vector`, `ds/hashmap`, `ds/ring_buffer`, `io/file`, `concurrency/thread_pool` |
| `_Thread_local` | Per-thread error context and the per-thread UUID PRNG | `sys/error_context`, `sys/uuid` |
| `_Static_assert` | Compile-time checks that the opaque platform storage is large enough | `concurrency/mutex`, `concurrency/semaphore` |
| `_Alignas` | Correctly aligning the opaque storage for the underlying platform primitives | `concurrency/mutex`, `concurrency/semaphore` |

Because atomics in particular cannot be emulated portably in C99, a C99 build of the whole
library is infeasible; partial C99 support would fragment the surface and is not offered.

## Compiler & platform matrix

| Platform | Architecture | Compiler | Minimum version | Notes |
|----------|--------------|----------|-----------------|-------|
| Linux    | x86_64       | GCC          | 11   | Primary CI toolchain |
| Linux    | x86_64       | Clang        | 14   | Sanitizer + pedantic CI toolchain |
| macOS    | arm64        | Apple Clang  | 14   | (Xcode 14+) |
| Windows  | x86_64       | MSVC (VS2022)| 19.30 | Needs `/experimental:c11atomics` for `<stdatomic.h>`; the build enables it automatically |

These cells are exactly the ones exercised by CI (`build`, `tsan`, `coverage`, `docs`,
`install`, `pedantic` jobs). Other modern C11 compilers and platforms are expected to work but
are not gated.

### MSVC and C11 atomics

MSVC gates `<stdatomic.h>` behind `/experimental:c11atomics`. The library's CMake target adds
this flag publicly, so any consumer that pulls in `ds/ring_buffer.h` (which uses C11 atomics)
also gets it. This requirement is expected to disappear once MSVC ships un-gated C11 atomics.

## Broad-reach conformance gate

To guarantee the library relies only on *standard* C11 — never a GCC/Clang/MSVC extension — CI
builds it with strict conformance settings:

- **GCC / Clang / Apple Clang:** `-std=c11 -pedantic-errors` (CMake `pedantic` preset /
  `D4NP_PEDANTIC=ON`), which turns any use of a non-ISO extension into a hard error.
- **MSVC:** `/std:c11 /permissive-`, which enforces conformance in the standard build matrix.

Reproduce the GCC/Clang gate locally:

```bash
cmake --preset pedantic
cmake --build --preset pedantic
```

A clean build across all listed compilers is the contract: the public surface is portable,
standard C11 with no extension creep.
