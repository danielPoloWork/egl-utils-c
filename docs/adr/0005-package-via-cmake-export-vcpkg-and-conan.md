# ADR-0005: Distribute via CMake install/export, a vcpkg port, and a Conan recipe

- **Status:** Accepted
- **Date:** 2026-06-21
- **Deciders:** Daniel Polo (maintainer), project architect
- **Related:** AGENTS.md §7 (docs/workflow/packaging.md), §11 (Versioning & Release), ROADMAP M8.3,
  ADR-0002 (module layout), ADR-0004 (docs)

## Context

`d4np-c` is meant to be *consumed*, not copy-pasted. Until now the build produced a library
target usable only from within this tree (`add_subdirectory`/`FetchContent`): there were no
install rules, no exported CMake package, and no presence in the two dominant C/C++ package
managers. `docs/workflow/packaging.md` described a publish flow in the abstract but named no
concrete mechanism.

Forces:

- **CMake-native consumption.** The canonical way to ship a CMake library is an installed
  *package config* so downstreams write `find_package(d4np-c) ; target_link_libraries(app
  d4np::d4np)` and inherit include dirs and the threads dependency transitively.
- **Two ecosystems, one source of truth.** vcpkg and Conan are both widely used; supporting
  both should not fork the build. Both can drive the project's own CMake install, so the CMake
  export is the foundation and the two package recipes are thin wrappers over it.
- **Header layout.** The umbrella header is included as `"d4np_c.h"` while module headers use the
  `"d4np/<module>/..."` prefix (ADR-0002). The install must reproduce exactly that so both
  include styles keep working from an install prefix.
- **Dependency surface.** The library links the platform threads library publicly; a consumer's
  `find_package` must re-resolve `Threads` or linking fails.
- **No release yet.** Version is `0.0.0` and nothing is tagged/published (see ADR-0003 context),
  so the recipes are authored as release-ready templates with a flagged placeholder source hash,
  and publishing stays human-gated per §11.

## Decision

Make the **CMake install/export the foundation** and layer the two package managers on top.

- A `D4NP_INSTALL` option (defaulting ON for a top-level build, OFF as a subproject) installs the
  `d4np` static library, lays the headers out as `include/d4np_c.h` + `include/d4np/<module>/*.h`,
  and exports the `d4np::d4np` target. `configure_package_config_file` +
  `write_basic_package_version_file` (using `GNUInstallDirs` and `CMakePackageConfigHelpers`)
  generate `d4np-cConfig.cmake` (which `find_dependency(Threads)`) and a `SameMajorVersion`
  version file under `lib/cmake/d4np-c`.
- A **vcpkg port** (`packaging/vcpkg/`) and a **Conan 2.x recipe** (`packaging/conan/`) both call
  the project's CMake with `D4NP_BUILD_TESTS=OFF D4NP_INSTALL=ON` and reuse the exported package;
  the Conan recipe re-derives its version from `version.h` and maps the package to the same
  `d4np::d4np` CMake target name.
- A standalone **consumer smoke-test** (`packaging/consumer-test/`) plus a Conan `test_package`
  exercise `find_package(d4np-c)` end to end. A CI `install` job runs the smoke-test on Linux,
  Windows, and macOS, proving the export works on every supported platform.

## Alternatives Considered

- **Header-only / vendored copy** — Rejected. The library has real `.c` translation units and a
  threads dependency; a package config models that correctly, and vendoring defeats versioned
  consumption.
- **Hand-written `d4np-cConfig.cmake`** — Rejected. `configure_package_config_file` produces a
  relocatable, `@PACKAGE_INIT@`-aware config; hand-rolling it invites broken relative paths.
- **vcpkg only, or Conan only** — Rejected. The two ecosystems serve different audiences and both
  ride the same CMake export at negligible extra cost.
- **Publish to registries from CI now** — Rejected. §11 makes publishing human-gated, and there
  is no reviewed mainline/tag to publish from yet; the recipes ship as ready templates.

## Consequences

- **API / compatibility:** additive — a new `D4NP_INSTALL` option and install artifacts; the
  default top-level build gains install rules, subproject use is unchanged (option defaults OFF).
- **Consumption:** `find_package(d4np-c CONFIG REQUIRED)` + `d4np::d4np`, or `vcpkg install`
  (overlay port) / `conan create packaging/conan`.
- **CI / tooling:** one new cross-platform `install` job. The vcpkg/Conan recipes are not yet
  exercised in CI (no published artifact); their foundation — the CMake export — is.
- **Risks / limitations:** the vcpkg `SHA512` and the `vX.Y.Z` source reference are placeholders
  until the first release tag; the Conan recipe is authored for Conan 2.x. These are finalized in
  the release milestone (M8.6) when a tag exists.

## References

- AGENTS.md §7, §11; ROADMAP.md M8.3; ADR-0002 (layout); `docs/workflow/packaging.md`.
- CMake `install(EXPORT)`, `CMakePackageConfigHelpers`, `GNUInstallDirs`.
- vcpkg maintainer guide (`vcpkg_cmake_configure`/`vcpkg_cmake_config_fixup`); Conan 2.x
  `CMakeToolchain`/`cmake_layout`.
