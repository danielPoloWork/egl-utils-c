# ADR-0006: Require C11 as the language floor; verify with a broad-reach pedantic build

- **Status:** Accepted
- **Date:** 2026-06-21
- **Deciders:** Daniel Polo (maintainer), project architect
- **Related:** AGENTS.md §1 (Persona — C11), §9 (Coding Conventions), ROADMAP M8.4,
  ADR-0005 (packaging)

## Context

ROADMAP item M8.4 was originally phrased as a **"C99-pedantic compatibility job (broad-reach
build)"** — the intent being to maximize the set of toolchains and standards that can build the
library. When implementing it, an audit of the actual sources showed the library depends
pervasively on C11 features with no portable pre-C11 equivalent:

- `<stdatomic.h>` / `_Atomic` in the lock-free SPSC structures (`atomic_queue`, `ring_buffer`)
  and `sys/log` — atomics are the heart of the concurrency value and **cannot** be emulated
  portably in C99.
- `<stdalign.h>` / `alignof` across eight translation units (allocators, containers, file I/O).
- `_Thread_local` (`error_context`, `uuid`), `_Static_assert` and `_Alignas` (`mutex`,
  `semaphore`).

A C99 build of the whole library is therefore **infeasible**, and a partial "C99 core" would
fragment the public surface for little benefit. This also matches AGENTS.md §1/§9, which already
designate C11 as the project's standard. The roadmap wording — written before the modules
landed — diverged from the implementation, and AGENTS.md §7 requires that divergence be recorded
in an ADR (and the plan corrected) rather than left to drift.

The real goal behind "broad-reach" is still worth gating: ensure the library uses only
*standard* C11, never a compiler-specific extension, so it ports cleanly across GCC, Clang,
Apple Clang, and MSVC.

## Decision

Adopt **C11 (ISO/IEC 9899:2011) as the hard language floor** and reinterpret M8.4 as a
**strict-C11 broad-reach conformance gate** rather than a C99 one.

- The library is built with `CMAKE_C_STANDARD 11` and `CMAKE_C_EXTENSIONS OFF` (so GCC/Clang
  select `-std=c11`, not `-std=gnu11`).
- A `D4NP_PEDANTIC` option (and a `pedantic` preset) adds `-pedantic-errors` on GCC/Clang,
  turning any use of a non-ISO extension into a hard compile error. The gate is scoped to
  *conformance*; it deliberately does not pile `-Werror` onto the wider warning set (that is a
  separate review-time concern).
- MSVC conformance is enforced by `/std:c11 /permissive-` in the existing build matrix.
- A CI `pedantic` job runs the strict build on GCC, Clang, and Apple Clang. The supported
  standard/compiler/platform set is captured in `docs/development/compatibility.md`.
- ROADMAP M8.4 is reworded from "C99-pedantic" to "strict-C11 broad-reach" (same item number).

## Alternatives Considered

- **Carve out a C99-buildable subset** (exclude the atomic modules; shim `alignof`,
  `_Static_assert`, `_Thread_local`) — Rejected. The atomics are central and still unavailable;
  the result would advertise a fractured, half-portable surface and add maintenance burden.
- **Refactor toward C99 with compatibility macros everywhere** — Rejected. Large, invasive churn
  that still cannot deliver standard atomics; it would trade real C11 clarity for an unreachable
  goal.
- **`-Werror` over the full `-Wall -Wextra -Wconversion` set in the gate** — Rejected for this
  ADR's scope. It conflates "uses a non-standard extension" (portability) with "is warning-free"
  (code hygiene); the latter belongs to the §10 review gate, not the conformance build.
- **Keep the C99 wording and skip the audit** — Rejected. It would leave a false compatibility
  claim in the roadmap and ship an untested promise.

## Consequences

- **API / compatibility:** no code change to the library; this formalizes the existing C11
  requirement and adds a build option + CI job. Consumers already needed a C11 compiler.
- **Documentation:** new `docs/development/compatibility.md` (the matrix); ROADMAP M8.4 reworded;
  README points at the matrix. C11 was already implied by AGENTS.md §1/§9.
- **CI / tooling:** one new `pedantic` job (3 cells). Contributors reproduce it with the
  `pedantic` preset.
- **Risks / limitations:** the gate proves *standard-C11 conformance*, not warning-freedom and
  not support on untested compilers. The MSVC `/experimental:c11atomics` requirement persists
  until MSVC un-gates C11 atomics. If a latent extension use exists, the first CI run will surface
  it as a real portability fix.

## References

- AGENTS.md §1, §7 (spec/plan divergence → ADR), §9; ROADMAP.md M8.4.
- `docs/development/compatibility.md`.
- GCC `-pedantic-errors` semantics (does not flag `__`-bracketed keywords such as
  `__attribute__`); MSVC `/permissive-`, `/experimental:c11atomics`.
