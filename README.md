# d4np-c

> a dependency-free, high-performance C11 systems utility library (allocators, containers, concurrency, strings, I/O, diagnostics)

![Status](https://img.shields.io/badge/Status-v0.0.0-blue)

A
library written in **C11**, built and governed to an enterprise quality
bar: full CI matrix, static analysis, sanitizers, documented design decisions, and SemVer
releases.

## What it is

>

The frozen specification is in
[`docs/specs/01_spec_d4np.md`](docs/specs/01_spec_d4np.md).

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
- **Supported platforms:** Linux x86_64 (GCC>=11, Clang>=14), Windows x86_64 (MSVC>=19.30), macOS arm64 (Apple Clang>=14).
- Consumers import the public surface via: `#include "d4np_c.h"`.

See [`docs/development/local-build.md`](docs/development/local-build.md) for the full local
setup.

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
