# Roadmap — d4np-c

The project's plan as a numbered, checkbox-driven list. When an item completes in a PR,
flip its checkbox (`- [ ]` → `- [x]`) **in the same PR**. New work goes at the bottom of
its section with a fresh `<milestone>.<task>` number; never renumber.

- **Versioning start:** pre-1.0 milestone-driven.
- **Session journal:** see [`docs/journal/`](docs/journal/). Latest checkpoint: _none yet_.

---

Milestones are slices of the spec, one functional module group per milestone, each shipping
green under the full quality bar (build + test + sanitizers + Valgrind, ≥80% coverage on the
code it adds). Numbers map to the 25 spec functions; `(#n)` references the spec item.

## Milestone 1 — Project bootstrap, foundation & first slice ✅

The thinnest slice that compiles, tests, and ships under the full quality bar.

- [x] 1.1 Lay down the build system (CMake + Ninja, CMakePresets) and the foundation under
      `d4np/core/` (`d4np_status_t`, pluggable `d4np_allocator_t` + default allocator).
- [x] 1.2 Implement the first end-to-end slice in `d4np/str/`: `d4np_str_view_t` +
      `d4np_str_view_split_next` (#15), incl. the spec §3 example.
- [x] 1.3 Wire the test framework (Unity, fetched) with passing tests under `tests/str/`.
- [x] 1.4 Add formatter + linter configs (clang-format, clang-tidy + cppcheck) at the repo root.
- [x] 1.5 Stand up the CI matrix (Linux/Windows/macOS) with build + test + format + lint + valgrind + tsan.
- [x] 1.6 Seed the version constant (D4NP_VERSION_*) in `d4np/core/version.h`.

## Milestone 2 — Memory & allocators (`d4np/mem`) ✅

- [x] 2.1 `d4np_arena_init` (#1) — initialize a bump-pointer arena over a backing block.
- [x] 2.2 `d4np_arena_alloc` (#2) — O(1) allocation with alignment honored.
- [x] 2.3 `d4np_arena_reset` (#3) — O(1) bulk free, keeping the main block (+ `d4np_arena_destroy`).
- [x] 2.4 `d4np_pool_init` (#4) — fixed-block pool over an intrusive free-list.
- [x] 2.5 `d4np_pool_alloc` / `d4np_pool_free` (#5) — O(1) block alloc/free, fragmentation-free.
- [x] 2.6 Benchmarks under `bench/mem` backing the O(1) claims (arena ~2 ns, pool ~3 ns vs malloc ~49 ns).

## Milestone 3 — Generic data structures (`d4np/ds`) ✅

- [x] 3.1 `d4np_vector_t` (#6) — dynamic array, geometric growth, allocator-injected.
- [x] 3.2 `d4np_hashmap_t` (#7) — open addressing + linear probing (cache-friendly).
- [x] 3.3 `d4np_linked_list_t` (#8) — intrusive doubly-linked list.
- [x] 3.4 `d4np_ring_buffer_t` (#9) — thread-safe SPSC circular buffer (verified under TSan).
- [x] 3.5 `d4np_string_builder_t` (#10) — efficient dynamic string assembly.
- [x] 3.6 Benchmarks under `bench/ds` (vector push ~6 ns, hashmap put/get ~90 ns, ring buffer push+pop ~7 ns).

## Milestone 4 — Concurrency & synchronization (`d4np/concurrency`) ✅

- [x] 4.1 `d4np_mutex_t` (#11) — portable shim over pthread/win32.
- [x] 4.2 `d4np_semaphore_t` (#14) — counting semaphore (in-process; named/IPC variant in M8.5).
- [x] 4.3 `d4np_atomic_queue_t` (#13) — lock-free SPSC over `<stdatomic.h>` (unbounded, node-based).
- [x] 4.4 `d4np_thread_pool_t` (#12) — native threads + internal task queue (graceful drain on shutdown).
- [x] 4.5 Stress tests + benchmarks under `bench/concurrency`; whole module green under TSan (mutex ~15 ns, atomic_queue enq+deq ~57 ns, pool dispatch ~0.5 M tasks/s).

## Milestone 5 — Strings & parsing (`d4np/str`) ✅

- [x] 5.1 `d4np_str_split` (#16) — zero-allocation split into a caller-provided view array.
- [x] 5.2 `d4np_str_parse_int` (#17) — overflow-safe integer parse returning `d4np_status_t`.
- [x] 5.3 `d4np_str_parse_float` (#17) — robust float parse with explicit error reporting.

## Milestone 6 — File system & I/O (`d4np/io`) ✅

- [x] 6.1 `d4np_file_read_all` (#18) — read a whole file into an allocator-backed buffer.
- [x] 6.2 `d4np_file_write_all` (#19) — atomic write (temp + fsync + rename).
- [x] 6.3 `d4np_path_combine` (#20) — OS-separator-aware safe path join.

## Milestone 7 — System utilities & diagnostics (`d4np/sys`) ✅

- [x] 7.1 `d4np_error_context_push` / `d4np_error_context_pop` (#22) — thread-local error trail.
- [x] 7.2 `d4np_log_write` (#21) — leveled logger (INFO/WARN/ERROR) to console or file.
- [x] 7.3 `d4np_timestamp_ms` (#23) — monotonic millisecond clock (+ ns).
- [x] 7.4 `d4np_uuid_generate` (#24) — RFC4122 v4 UUID (+ canonical formatting).
- [x] 7.5 `d4np_hash_fnv1a` (#25) — FNV-1a over strings and binary buffers (+ continuation).

## Milestone 8 — Hardening & 1.0 release

- [x] 8.1 Coverage gate ≥80% line across all modules (llvm-cov/gcovr) enforced in CI.
- [ ] 8.2 Doxygen API docs published; README quickstart per module group.
- [ ] 8.3 Packaging: vcpkg port + Conan recipe; install/export CMake targets.
- [ ] 8.4 C99-pedantic compatibility job (broad-reach build) and a documented compat matrix.
- [ ] 8.5 Multi-process test harness for `d4np_semaphore_t` IPC; perf thresholds gated in CI.
- [ ] 8.6 Cut `1.0.0` — freeze the public ABI, write release notes, tag.

---

## Spec Coverage Map

Tracks which spec section is fulfilled by which milestone(s). Every spec section has a
row with at least one fulfilling milestone and a status glyph. Legend: ⏳ not started · 🚧 in
progress · ✅ done · ❎ N/A.

| Spec § | Requirement | Milestones | Status |
|--------|-------------|------------|--------|
| §1 | Objective & business context | M1 | ✅ |
| §2 | Functional requirements (25 functions) | M1–M7 | 🚧 |
| §3 | Non-functional requirements (perf, no-leak, thread-safety) | M2, M4, M8 | 🚧 |
| §4 | Logical architecture | M1 | ✅ |
| §5 | Public interface | M1–M7 | 🚧 |
| §6 | Verification & test strategy | M1, M4, M8 | 🚧 |

> **Status:** Milestone 1 complete — foundation (`d4np/core`) + the `d4np/str` string-view
> slice build and test green (MSVC verified locally; full Tier-1 matrix runs in CI). The
> remaining 23 spec functions land across M2–M7, with M8 hardening to 1.0.
