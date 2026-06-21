# ADR-0008: Freeze the public API/ABI for the 1.x line

- **Status:** Accepted
- **Date:** 2026-06-21
- **Deciders:** Daniel Polo (maintainer), project architect
- **Related:** AGENTS.md §11 (Versioning & Release), ROADMAP M8.6, docs/workflow/maintenance.md,
  ADR-0002 (module layout), ADR-0005 (packaging)

## Context

`d4np-c` is cutting `1.0.0` — its first stable release. Adopting Semantic Versioning at 1.0
turns the public surface into a contract: downstreams that pin `^1` expect every 1.x upgrade to
be drop-in. Until now the project was pre-1.0 milestone-driven and made no compatibility promise,
so the boundary of "what is frozen" was never stated.

For a C library this is sharper than for most languages because compatibility has **two** layers:

- **API** (source) — types, function signatures, macro names, enum spellings: a consumer must
  keep compiling.
- **ABI** (binary) — struct sizes and layouts, enum *values*, calling-relevant types: a consumer
  must keep linking and running against a differently-built `1.y` without recompiling.

Several public types deliberately expose **opaque fixed-size storage** (e.g.
`d4np_mutex_t`, `d4np_semaphore_t`, `d4np_named_semaphore_t` carry `_Alignas(16) unsigned char
opaque[N]`). The size `N` is part of the ABI: a consumer embeds these by value, so changing `N`
silently breaks every caller's struct layout. This must be governed explicitly.

## Decision

From `1.0.0`, the **public API and ABI are frozen for the entire 1.x line** under SemVer 2.0.0.

The frozen surface is everything reachable from `d4np_c.h` (the per-module public headers):

- function signatures and their documented semantics / error contracts;
- public type definitions and struct layouts, **including the `opaque[N]` storage sizes and
  `_Alignas` of the concurrency primitives**;
- enum constants **and their numeric values** (`d4np_status_t`, `d4np_log_level_t`, …);
- public macro names and meanings;
- the CMake package name (`d4np-c`) and exported target (`d4np::d4np`).

Within 1.x:

- **PATCH** — bug/doc/packaging/perf fixes with no surface change.
- **MINOR** — purely additive: new functions, new types, new enum constants *appended* (never
  renumbered), new opt-in options. Existing layouts and values are untouched.
- **MAJOR (2.0.0)** — required for any removal, rename, signature/semantics change, enum
  renumbering, or any growth/shrink of an `opaque[N]` size. Each such break needs its own ADR and
  a migration note, and goes through the deprecation window in `maintenance.md`.

Opaque storage sizes are sized with headroom now precisely so MINOR additions to the backing
implementation do not force an ABI break.

## Alternatives Considered

- **API-only stability (let ABI drift)** — Rejected. The opaque-by-value types make ABI breaks
  invisible at compile time but fatal at runtime; promising only source compatibility would set a
  trap for anyone shipping `d4np-c` as a shared/transitive dependency.
- **Hide all state behind heap-allocated handles to dodge ABI concerns** — Rejected. It would
  defeat the zero-allocation, embed-by-value design (a core value of the allocators/primitives)
  for a guarantee SemVer already lets us make explicitly.
- **Stay pre-1.0 indefinitely** — Rejected. The surface is complete and verified; consumers need
  a stable contract, which is what 1.0 communicates.

## Consequences

- **For consumers:** anything pinned to `^1` compiles, links, and runs across 1.x upgrades.
- **For maintainers:** the `maintenance.md` decision tree gains a hard rule — touching an
  `opaque[N]` size, an enum value, or a signature is a 2.0 event. The `_Static_assert`s that guard
  the opaque sizes also catch an accidental overgrowth at compile time.
- **Risks / limitations:** ABI stability is asserted per the contract above and guarded by the
  static asserts and the compatibility matrix, but is not yet checked by an automated ABI-diff
  tool (e.g. `abidiff`); adding one is a candidate for a future hardening item.

## References

- AGENTS.md §11; docs/workflow/maintenance.md (SemVer decision tree, deprecation policy).
- Semantic Versioning 2.0.0 (<https://semver.org/>).
- ADR-0002 (opaque storage / module layout); concurrency headers' `opaque[N]` + `_Static_assert`.
