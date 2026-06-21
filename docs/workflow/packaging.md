# Packaging & Distribution

How `d4np-c` is built into a distributable artifact and published. This doc exists
because the project is distributed via a package registry (`capabilities.packaging`).

## Artifact

- **What ships:** the package produced by `CMake + Ninja` (and its contents — runtime only,
  no tests/benches).
- **Consumers import it via:** `#include "d4np_c.h"`.
- **Metadata:** name, version (from `version.h`), license `MIT`, and the
  links a registry expects (repo, docs, changelog).

## Distribution mechanisms

The CMake install/export is the foundation; the two package managers wrap it (see
[ADR-0005](../adr/0005-package-via-cmake-export-vcpkg-and-conan.md)).

### CMake install / `find_package`

```bash
cmake -S . -B build -DD4NP_INSTALL=ON
cmake --build build
cmake --install build --prefix /your/prefix
```

Downstream:

```cmake
find_package(d4np-c CONFIG REQUIRED)
target_link_libraries(app PRIVATE d4np::d4np)
```

The package installs `include/d4np_c.h` (umbrella) + `include/d4np/<module>/*.h`, the `d4np`
static library, and `lib/cmake/d4np-c/` (config + version + targets). The config calls
`find_dependency(Threads)`. The `packaging/consumer-test/` project and the CI `install` job
verify this end to end on Linux, Windows, and macOS.

### vcpkg (`packaging/vcpkg/`)

An overlay port until upstreamed:

```bash
vcpkg install d4np-c --overlay-ports=packaging/vcpkg
```

The port runs the project's own CMake install and fixes up the package config. Its `SHA512`
and source ref are placeholders until the first `vX.Y.Z` tag (see the portfile note).

### Conan (`packaging/conan/`)

A Conan 2.x recipe that builds from the repository root and reuses the CMake export:

```bash
conan create packaging/conan --build=missing
```

It derives its version from `version.h`, maps to the `d4np::d4np` CMake target, and adds the
system `pthread` dependency on Linux. `packaging/conan/test_package/` validates consumption.

## Registry

- **Where:** the package registry.
- **Auth:** publishing credentials live in CI secrets, never in the repo.

## Publish flow

Publishing is tied to the release ([`release.md`](release.md)) and is a **human-gated** step,
like the GitHub Release:

1. The release PR is merged and the annotated tag is pushed (`vX.Y.Z`).
2. CI builds the artifact on the tag and verifies it (contents, metadata, version match).
3. A human approves the publish step; CI pushes to the registry.
4. The published version is immutable — a mistake is fixed forward with a new version, never by
   overwriting.

## Versioning & provenance

- The published version **equals** the tag and the version constant (the consistency lint's
  `version-lockstep` enforces this).
- Prefer a reproducible build and attach provenance/SBOM where the ecosystem supports it.
