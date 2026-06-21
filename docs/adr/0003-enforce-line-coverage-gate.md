# ADR-0003: Enforce an 80% line-coverage gate via gcovr in CI

- **Status:** Accepted
- **Date:** 2026-06-21
- **Deciders:** Daniel Polo (maintainer), project architect
- **Related:** AGENTS.md §10 (Enterprise Quality Bar), ROADMAP M8.1, spec §6 (Verification & test strategy)

## Context

The quality bar in `AGENTS.md` §10 states that new code must reach **≥80% line coverage**,
explicitly deferring the concrete mechanism to "an ADR". Through milestones M1–M7 the library
grew to 21 implementation units across seven modules with a Unity test suite, but coverage was
never *measured* — let alone *enforced*. Without an automated gate, the 80% requirement is an
aspiration that silently erodes: a PR can add an untested error branch and still go green.

Forces at play:

- **C toolchain reality.** Line coverage in C is produced by instrumenting builds. The two
  mainstream front-ends are GCC's `gcov` and Clang's source-based coverage (`llvm-cov`). Both
  emit counters that `gcovr` can aggregate into a single threshold check and human-readable
  reports.
- **Cross-platform matrix.** Our CI spans GCC (Linux), Clang (Linux sanitizers), Apple Clang
  (macOS), and MSVC (Windows). Coverage instrumentation via `--coverage` is a GCC/Clang
  feature; MSVC has no comparable free, scriptable line-coverage path. The gate therefore must
  run on a single, deterministic cell rather than the full matrix.
- **Scope of measurement.** The number that matters is coverage of the *library* (`d4np/`),
  not of the test harness or fetched dependencies (Unity). Instrumenting everything and
  reporting a blended figure would be misleading.
- **Reproducibility.** Per §10, every enforced claim must be reproducible locally, not only in
  CI, so a contributor can see the same number before pushing.

## Decision

Adopt **gcovr** as the coverage driver and enforce a **minimum of 80% line coverage over the
`d4np/` source tree** as a required CI job.

- A new opt-in CMake option `D4NP_ENABLE_COVERAGE` (GCC/Clang only; a hard `FATAL_ERROR` on
  MSVC) instruments the library with `--coverage -O0 -g` and propagates the `--coverage` link
  flag to the test executable so `.gcda` counters are emitted on exit. A `coverage` preset
  wires it up.
- `tools/coverage.sh [THRESHOLD]` (default `80`) is the single reproducible entry point: it
  configures/builds/tests the `coverage` preset and runs `gcovr --filter 'd4np/'
  --fail-under-line "$THRESHOLD"`, emitting Cobertura XML and a browsable HTML report under
  `coverage/`. It honours a `GCOV` env var so a Clang build can pass `GCOV="llvm-cov gcov"`.
- CI runs the gate once, on `ubuntu-24.04` with GCC, via a `coverage` job that fails below the
  threshold and uploads the HTML report as an artifact.

The threshold is **80%**, matching §10. It is a floor, not a target: modules are expected to
sit well above it, and the number may be raised in a future ADR as the suite matures.

## Alternatives Considered

- **`llvm-cov` (source-based coverage) directly** — Rejected as the *driver*. It produces
  excellent region/branch data but is Clang-specific and its reports are harder to aggregate to
  a single cross-tool threshold. gcovr sits above both gcov and `llvm-cov gcov`, so we keep the
  option open (via the `GCOV` env var) without coupling the gate to one compiler.
- **lcov + genhtml** — Rejected. Perl-based, slower, and historically brittle with recent GCC
  `.gcno` formats; gcovr is a single `pip install`, aligning with our Python-only tooling
  (`consistency_lint.py`).
- **Enforce coverage across the full build matrix** — Rejected. MSVC has no free, scriptable
  line-coverage path, and Apple Clang/Linux Clang would only duplicate the GCC number at triple
  the CI cost. One deterministic cell is sufficient and faster.
- **Track coverage without failing the build (report-only)** — Rejected. §10 forbids
  "tests next PR" shortcuts; an unenforced metric is exactly that. The gate must be blocking.

## Consequences

- **API / compatibility:** none. `D4NP_ENABLE_COVERAGE` defaults `OFF`; consumers and the
  default presets are unaffected, and the library still depends only on libc + threads.
- **Testing & tooling:** a new required CI job (~one extra Linux build) and a new local script.
  Contributors run `tools/coverage.sh` to reproduce the CI number. `gcovr` joins the
  documented local-build dependencies.
- **Performance trade-offs:** coverage builds are `-O0` and slower, but they are isolated to
  the `coverage` preset/job; the functional and sanitizer cells are untouched.
- **Documentation:** README build section and `docs/development/local-build.md` gain the
  coverage command; ROADMAP M8.1 is satisfied by this gate.
- **Risks / limitations:** the gate measures **line** coverage only (not branch/MC-DC); it runs
  on GCC alone, so a compiler-specific dead branch elsewhere would go unmeasured. Header-only
  inline code is included via the `d4np/` filter. Raising the bar or adding branch coverage is
  left to a future ADR.

## References

- AGENTS.md §10 (Enterprise Quality Bar), §6.4 (pre-PR congruence check).
- ROADMAP.md M8.1.
- gcovr documentation: <https://gcovr.com/>.
- GCC `--coverage` / gcov; LLVM source-based coverage (`llvm-cov gcov`).
