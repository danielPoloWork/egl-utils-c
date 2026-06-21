# d4np-c

> a dependency-free, high-performance C11 systems utility library (allocators, containers, concurrency, strings, I/O, diagnostics)

![Status](https://img.shields.io/badge/Status-v0.0.0-blue)

A
library written in **C11**, built and governed to an enterprise quality
bar: full CI matrix, static analysis, sanitizers, documented design decisions, and SemVer
releases.

## What it is

A single, dependency-free toolbox of the building blocks C systems code rewrites on every
project — allocators, containers, concurrency primitives, string/parse helpers, file I/O, and
diagnostics — each allocator-injected, error-checked, and verified under sanitizers. The
library links against nothing but libc and the platform threads library.

The frozen specification is in
[`docs/specs/01_spec_d4np.md`](docs/specs/01_spec_d4np.md). The full API reference is generated
with Doxygen (`doxygen docs/Doxyfile` → `docs/doxygen/html/`).

## Build, test, run

```bash
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

Line coverage (GCC/Clang; enforces the ≥80% gate over `d4np/`, see
[ADR-0003](docs/adr/0003-enforce-line-coverage-gate.md)):

```bash
tools/coverage.sh        # build + test instrumented, then gcovr --fail-under-line 80
```

- **Toolchain:** CMake + Ninja, Unity, clang-format, clang-tidy + cppcheck.
- **Supported platforms:** Linux x86_64 (GCC>=11, Clang>=14), Windows x86_64 (MSVC>=19.30), macOS arm64 (Apple Clang>=14). Language floor is **C11** — see the [compatibility matrix](docs/development/compatibility.md).
- Consumers import the public surface via: `#include "d4np_c.h"`.

See [`docs/development/local-build.md`](docs/development/local-build.md) for the full local
setup.

## Install & consume

Install the library and its CMake package, then depend on it with `find_package`:

```bash
cmake -S . -B build -DD4NP_INSTALL=ON
cmake --build build
cmake --install build --prefix /your/prefix
```

```cmake
find_package(d4np-c CONFIG REQUIRED)
target_link_libraries(app PRIVATE d4np::d4np)
```

A [vcpkg port](packaging/vcpkg/) and a [Conan recipe](packaging/conan/) wrap the same export —
see [`docs/workflow/packaging.md`](docs/workflow/packaging.md).

## Quickstart by module group

Include the whole surface with `#include "d4np_c.h"`, or one module via its public header.
Error handling is elided below for brevity — every fallible call returns a `d4np_status_t`
(`D4NP_OK == 0`) or a documented `NULL`/sentinel.

**`core`** — pluggable allocator + status codes:

```c
#include "d4np/core/d4np_allocator.h"
const d4np_allocator_t *a = d4np_allocator_default();
void *p = d4np_alloc(a, 256, 16);
d4np_free(a, p, 256);
```

**`mem`** — arena (bump-pointer) and fixed-block pool allocators:

```c
#include "d4np/mem/arena.h"
d4np_arena_t arena;
d4np_arena_init(&arena, NULL, 64 * 1024);   /* NULL -> default allocator */
char *buf = d4np_arena_alloc(&arena, 128, 16);
d4np_arena_reset(&arena);                    /* O(1) bulk free */
d4np_arena_destroy(&arena);
```

**`ds`** — vector, hash map, linked list, ring buffer, string builder:

```c
#include "d4np/ds/vector.h"
d4np_vector_t v;
d4np_vector_init(&v, NULL, sizeof(int), 0);
int x = 42;
d4np_vector_push(&v, &x);
int *first = d4np_vector_at(&v, 0);
d4np_vector_destroy(&v);
```

**`concurrency`** — mutex, semaphore, lock-free SPSC queue, thread pool:

```c
#include "d4np/concurrency/thread_pool.h"
static void work(void *arg) { (void)arg; /* ... */ }
d4np_thread_pool_t pool;
d4np_thread_pool_init(&pool, NULL, 4);
d4np_thread_pool_submit(&pool, work, NULL);
d4np_thread_pool_destroy(&pool);             /* graceful drain + join */
```

**`str`** — string views, splitting, overflow-safe parsing:

```c
#include "d4np/str/parse.h"
d4np_str_view_t sv = d4np_str_view_from_str("42");
int64_t n;
if (d4np_str_parse_int(sv, 0, &n) == D4NP_OK) { /* n == 42 */ }
```

**`io`** — whole-file read/write (durable, atomic) + path joining:

```c
#include "d4np/io/file.h"
#include "d4np/io/path.h"
char path[256];
d4np_path_combine(path, sizeof path, "/tmp", "out.bin");
d4np_file_write_all(path, "hi", 2);          /* temp + fsync + rename */
unsigned char *data; size_t len;
d4np_file_read_all(path, NULL, &data, &len);
d4np_free(NULL, data, len);
```

**`sys`** — logger, error context, monotonic clock, UUIDv4, FNV-1a:

```c
#include "d4np/sys/log.h"
#include "d4np/sys/uuid.h"
d4np_log_write(D4NP_LOG_INFO, "user %d connected", 7);
d4np_uuid_t id; char text[D4NP_UUID_STRING_LEN];
d4np_uuid_generate(&id);
d4np_uuid_format(&id, text);
```

## How this project is run

| Document | Purpose |
|---|---|
| [`AGENTS.md`](AGENTS.md) | How AI agents (and humans) work in this repo — the contract. |
| [`ROADMAP.md`](ROADMAP.md) | The numbered plan and what is done. |
| [`docs/adr/`](docs/adr/) | Why it is built the way it is (Architecture Decision Records). |
| [`docs/patterns/`](docs/patterns/) | Design patterns adopted, rejected, or considered. |
| [`docs/workflow/`](docs/workflow/) | Git, documentation, release, and maintenance conventions. |
| [`CHANGELOG.md`](CHANGELOG.md) | User-visible changes per release. |
| [`SECURITY.md`](SECURITY.md) | How to report a vulnerability. |

## Milestones

| # | Title | Status |
|---|---|---|
| 1 | Project bootstrap & CI | ⏳ in progress |

## License

MIT © 2026 Daniel Polo. See [`LICENSE`](LICENSE).
