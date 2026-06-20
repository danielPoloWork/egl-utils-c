# ADR-0002: Adopt a module-oriented source layout

- **Status:** Accepted
- **Date:** 2026-01-01
- **Deciders:** Maintainer
- **Related:** ADR-0001, AGENTS.md §5

## Context

`d4np-c` is a dependency-free C11 systems utility library whose public surface is grouped
into cohesive functional modules (memory/allocators, containers, concurrency, strings,
file-system I/O, and system diagnostics). The EAAO factory's default is a Maven-style
cross-language tree (`src/main/<lang>/<group>/<slug>/`), which optimizes for identical shape
across many sibling languages. For a single-language C library that consumers integrate by
`#include`, that deep nested path adds friction (`src/main/c/it/d4np/d4np/`) without buying
anything: there is no second language segment to disambiguate, and C has no package roots —
only include paths. The maintainer therefore chose a flat, module-first layout.

## Decision

Production sources live under a single root folder named after the library, subdivided **by
module**:

```text
d4np/                  # production sources (public + internal headers and .c files)
├── core/              # foundation: d4np_status_t, d4np_allocator_t, error context, version.h
├── mem/               # arena and fixed-block pool allocators
├── ds/                # vector, hashmap, linked_list, ring_buffer, string_builder
├── concurrency/       # mutex, semaphore, thread_pool, atomic SPSC queue
├── str/               # str_view, split, parse_int/float
├── io/                # file_read_all/write_all, path_combine
├── sys/               # log, error_context, timestamp, uuid, hash_fnv1a
└── d4np_c.h           # umbrella public header (includes the per-module public headers)

tests/<module>/        # Unity unit tests, mirroring the module folders
bench/<module>/        # benchmarks, mirroring the module folders (where applicable)
```

Subdivision inside `d4np/` is **by module (component)**, not by file type: a module's public
header, internal headers, and implementation `.c` files sit together in its folder. Consumers
import the whole surface via the single umbrella header `#include "d4np_c.h"`, or a single
module via its public header (e.g. `#include "d4np/str/str_view.h"`).

## Alternatives Considered

- **The EAAO default `src/main/<lang>/<group>/<slug>/` tree.** Rejected for this repository —
  it exists to keep *many* language repos identically shaped; this is a single-language C
  library where the nesting is pure overhead and unidiomatic for C consumers.
- **Layout by file type (`include/` + `src/`).** Rejected — splitting a module's header from
  its implementation across two top-level trees scatters a single concern and complicates
  navigation; co-locating per module keeps each component self-contained.
- **A fully flat `d4np/` with no submodules.** Rejected — the spec's 25 functions group
  naturally into six modules; flat would obscure those boundaries.

## Consequences

- CMake treats `d4np/` as the source root and exposes the repo root on the include path so
  both `#include "d4np_c.h"` and `#include "d4np/<module>/<header>.h"` resolve.
- The version constant (`D4NP_VERSION_*`) lives in `d4np/core/version.h`.
- The consistency lint's `src_main` root is `d4np/`; pattern code-locations and tests are
  checked against `d4np/`, `tests/`, and `bench/`.
- The layout is enforceable: production code outside `d4np/` is a review failure, and changing
  the shape requires superseding this ADR.

## References

- AGENTS.md §5 (Source Tree & Layout).
- spec: `docs/specs/01_spec_d4np.md` (the six functional modules).
