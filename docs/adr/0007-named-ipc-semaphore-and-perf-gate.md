# ADR-0007: Add a named/IPC semaphore and a loose-threshold performance gate

- **Status:** Accepted
- **Date:** 2026-06-21
- **Deciders:** Daniel Polo (maintainer), project architect
- **Related:** ROADMAP M8.5, M4.2 (in-process semaphore), spec §3 (non-functional: perf,
  thread-safety), ADR-0006 (C11 floor)

## Context

Two M8.5 deliverables share this record because they were designed together.

1. **Cross-process semaphore.** M4 shipped `d4np_semaphore_t` as an *in-process* counting
   semaphore (Win32 `CreateSemaphore`; POSIX mutex+cond, since *unnamed* `sem_t` is unusable on
   macOS). The named/IPC variant was explicitly deferred to M8. Inter-process synchronization is a
   distinct capability — different kernel object, different lifetime, different failure modes — and
   needs its own type and, crucially, a **multi-process** test, which the in-process Unity harness
   cannot provide.

2. **Performance gating.** The spec's non-functional requirements make performance claims, and
   `bench/concurrency` already *reports* hot-path costs, but nothing *enforced* them: a regression
   that, say, slipped a syscall into `mutex_lock` would pass CI silently. M8.5 asks for "perf
   thresholds gated in CI". The tension is that CI runners are shared and noisy, so a tight
   threshold would flake constantly.

## Decision

**Named/IPC semaphore.** Add `d4np_named_semaphore_t` (`concurrency/named_semaphore.h`) with
`open` (create-or-open), `close`, `unlink`, `wait`, `trywait`, `post`. It wraps POSIX `sem_open`
(Linux + macOS — *named* POSIX semaphores work on both, unlike unnamed) and the Windows named
`CreateSemaphore`. Callers pass a short bare key; the implementation maps it to the platform
namespace (leading `/` on POSIX) and caps it to fit macOS's 31-char limit. Lifetime mirrors the
POSIX model: `unlink` removes the name (a no-op on Windows, where the object dies with its last
handle). Verification is a standalone harness (`tests/concurrency/named_sem_harness.c`) that
re-launches itself (`fork`+`execv` / `CreateProcess`) and runs a 2000-round bidirectional
ping-pong over two named semaphores; because each process opens the objects independently *by
name*, success proves real kernel-backed IPC. It runs as the `d4np_named_sem_ipc` CTest with a
60s TIMEOUT so a broken build fails fast instead of hanging.

**Performance gate.** Add `bench/concurrency/perf_gate.c`, which measures the uncontended hot
paths (mutex lock+unlock, semaphore post+trywait, atomic-queue enqueue+dequeue) and exits
non-zero if any exceeds a ceiling. Ceilings are deliberately **loose** — ~20-50x the figures
measured on a developer machine — so the gate catches only catastrophic regressions, never runner
jitter. The CI `benchmark` job builds the (Release) bench suite and runs the gate.

## Alternatives Considered

- **Extend `d4np_semaphore_t` with an optional name** — Rejected. It would bloat the in-process
  fast path's storage and conflate two lifetimes (memory-scoped vs. kernel-named). A separate type
  keeps each contract clean.
- **POSIX unnamed `sem_t` in shared memory for IPC** — Rejected. `sem_init(pshared=1)` plus an
  `mmap`'d `MAP_SHARED` region is more moving parts than `sem_open`, and unnamed semaphores are
  unsupported on macOS — the very reason M4 avoided them.
- **Run the multi-process test inside the Unity binary via `fork`** — Rejected. A standalone
  binary is cleaner across Windows (no `fork`) and keeps the Unity run single-process and fast.
- **Tight, tracked performance thresholds (e.g. fail on >10% regression vs. a baseline)** —
  Rejected for now. It needs a stored per-runner baseline and would flake on shared CI; the loose
  absolute ceiling is the robust first step. A tracked baseline can come later.

## Consequences

- **API:** additive — a new public type and header, exported through the umbrella header and the
  installed package. No change to existing types.
- **Testing/CI:** a new `d4np_named_sem_ipc` CTest (multi-process) and an enforced perf gate in the
  `benchmark` job. The in-process unit tests also cover the new API's non-blocking paths.
- **Portability:** `sem_open` name limits differ across POSIX systems; the 31-char cap and
  `/`-prefix normalization keep keys portable. Windows `unlink` is a documented no-op.
- **Risks / limitations:** the IPC harness depends on `argv[0]`/`GetModuleFileName` resolving the
  binary for re-launch (true under CTest); a crashed run may leave a named object behind on POSIX,
  so the parent pre-unlinks. The perf ceilings trade sensitivity for stability — they will not
  catch small regressions by design.

## References

- ROADMAP.md M8.5; M4.2 note (named/IPC variant deferred).
- POSIX `sem_open`/`sem_unlink`; Windows `CreateSemaphore` named objects.
- `docs/development/compatibility.md` (platform matrix).
