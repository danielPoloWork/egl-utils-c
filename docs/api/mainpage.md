# d4np-c API Reference {#mainpage}

**d4np-c** is a dependency-free, high-performance C11 systems utility library: allocators,
containers, concurrency primitives, strings, I/O, and diagnostics. The library itself depends
only on the C standard library and the platform threads library.

Include the whole public surface with a single header:

```c
#include "d4np_c.h"
```

…or pull in just one module:

```c
#include "d4np/mem/arena.h"
```

## Modules

The API is organized into the module groups below — see the **Modules** tab for the full
per-function reference.

| Group | Header prefix | What it provides |
|-------|---------------|------------------|
| @ref d4np_core         | `d4np/core/`        | Status codes, pluggable allocator, version constants |
| @ref d4np_mem          | `d4np/mem/`         | Arena (bump-pointer) and fixed-block pool allocators |
| @ref d4np_ds           | `d4np/ds/`          | Vector, hash map, intrusive list, ring buffer, string builder |
| @ref d4np_concurrency  | `d4np/concurrency/` | Mutex, counting semaphore, lock-free SPSC queue, thread pool |
| @ref d4np_str          | `d4np/str/`         | String views, splitting, overflow-safe number parsing |
| @ref d4np_io           | `d4np/io/`          | Whole-file read/write, OS-aware path joining |
| @ref d4np_sys          | `d4np/sys/`         | Logger, error context, monotonic clock, UUIDv4, FNV-1a |

## Conventions

- **Error model.** Every fallible function returns a ::d4np_status_t; `D4NP_OK` (zero) is
  success, so callers can write `if (rc != D4NP_OK)`. Functions that return a pointer or value
  signal failure with `NULL` / a documented sentinel.
- **Ownership.** Allocating types take a `::d4np_allocator_t` (NULL selects the default); the
  caller owns what it passes in and is responsible for the matching `*_destroy`.
- **Thread-safety.** Each type documents its guarantees explicitly; look for the **Note**
  callout on individual functions.

See the project [README](https://github.com/danielPoloWork/egl-utils-c) and `ROADMAP.md` for
build instructions and project status.
