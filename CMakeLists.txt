# d4np-c — dependency-free C11 systems utility library.
# SPDX-License-Identifier: MIT
#
# The library target itself depends only on the C standard library (and the platform threads
# library for the concurrency module). Test/benchmark dependencies are fetched only when the
# corresponding option is ON, so a consumer who vendors d4np-c pays for nothing extra.
cmake_minimum_required(VERSION 3.20)

project(d4np-c
    VERSION 0.0.0
    DESCRIPTION "Dependency-free, high-performance C11 systems utility library"
    LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(D4NP_BUILD_TESTS "Build the d4np-c unit tests" ON)
option(D4NP_BUILD_BENCH "Build the d4np-c benchmarks" OFF)
option(D4NP_ENABLE_COVERAGE "Instrument the library and tests for line coverage (GCC/Clang only)" OFF)
option(D4NP_PEDANTIC "Build the library as strict ISO C11 with warnings-as-errors (broad-reach gate)" OFF)
# Default the install/export rules ON for a top-level build, OFF when d4np-c is pulled in as a
# subproject (add_subdirectory / FetchContent), so a consumer's `install` stays uncluttered.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(D4NP_INSTALL_DEFAULT ON)
else()
    set(D4NP_INSTALL_DEFAULT OFF)
endif()
option(D4NP_INSTALL "Generate install and CMake-package export rules for d4np-c" ${D4NP_INSTALL_DEFAULT})

# The concurrency module wraps pthread/win32; pull in the platform threads library now so the
# public target carries it for consumers.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------
# Library
# ---------------------------------------------------------------------------
add_library(d4np
    d4np/core/d4np_status.c
    d4np/core/d4np_allocator.c
    d4np/mem/arena.c
    d4np/mem/pool.c
    d4np/ds/vector.c
    d4np/ds/string_builder.c
    d4np/ds/hashmap.c
    d4np/ds/linked_list.c
    d4np/ds/ring_buffer.c
    d4np/concurrency/mutex.c
    d4np/concurrency/semaphore.c
    d4np/concurrency/atomic_queue.c
    d4np/concurrency/thread_pool.c
    d4np/str/str_view.c
    d4np/str/parse.c
    d4np/io/file.c
    d4np/io/path.c
    d4np/sys/hash.c
    d4np/sys/clock.c
    d4np/sys/uuid.c
    d4np/sys/error_context.c
    d4np/sys/log.c)
add_library(d4np::d4np ALIAS d4np)

target_include_directories(d4np
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/d4np>
        $<INSTALL_INTERFACE:include>)

# The public headers use C11 (atomics, _Alignas); requiring it PUBLIC propagates the standard to
# downstream find_package consumers (e.g. MSVC needs /std:c11 to parse _Alignas in mutex.h).
target_compile_features(d4np PUBLIC c_std_11)

target_link_libraries(d4np PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(d4np PRIVATE /W4 /permissive-)
    # MSVC gates C11 <stdatomic.h> behind this switch; PUBLIC so consumers that include the
    # ring_buffer header (which uses C11 atomics) also enable it. Drop once MSVC un-gates it.
    target_compile_options(d4np PUBLIC /experimental:c11atomics)
    # Broad-reach (M8.4): MSVC conformance is already enforced by /permissive- above plus
    # /std:c11 (CMAKE_C_STANDARD 11 + CMAKE_C_EXTENSIONS OFF); D4NP_PEDANTIC adds nothing here.
else()
    target_compile_options(d4np PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wstrict-prototypes)
    # Broad-reach gate (M8.4): pure ISO C11 (CMAKE_C_EXTENSIONS OFF -> -std=c11). -pedantic-errors
    # makes any use of a GNU/compiler extension a hard error, proving the library is strictly
    # conforming. Scoped to conformance — it deliberately does not add -Werror over the wider
    # warning set. See ADR-0006 and docs/development/compatibility.md.
    if(D4NP_PEDANTIC)
        target_compile_options(d4np PRIVATE -pedantic-errors)
    endif()
endif()

# ---------------------------------------------------------------------------
# Coverage instrumentation (M8.1) — GCC/Clang only, opt-in via the `coverage`
# preset. We instrument the library sources (PRIVATE compile flags) and propagate
# the link flag PUBLICly so any consumer that links d4np — notably d4np_tests —
# emits the .gcda counters at exit. gcovr (see tools/coverage.sh) then enforces
# the >=80% line gate over d4np/ only. See ADR-0003.
# ---------------------------------------------------------------------------
if(D4NP_ENABLE_COVERAGE)
    if(NOT (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang"))
        message(FATAL_ERROR
            "D4NP_ENABLE_COVERAGE requires GCC or Clang (got ${CMAKE_C_COMPILER_ID}).")
    endif()
    target_compile_options(d4np PRIVATE --coverage -O0 -g)
    target_link_options(d4np PUBLIC --coverage)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        # Emit absolute paths in the .gcno/.gcda so gcovr resolves sources from any CWD.
        target_compile_options(d4np PRIVATE -fprofile-abs-path)
    endif()
endif()

# ---------------------------------------------------------------------------
# Install & CMake-package export (M8.3) — makes `find_package(d4np-c)` work for
# downstream consumers (incl. the vcpkg port and Conan recipe under packaging/).
# See ADR-0005.
# ---------------------------------------------------------------------------
if(D4NP_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # The compiled library + its usage requirements (the INSTALL_INTERFACE include dir).
    install(TARGETS d4np
        EXPORT d4np-cTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    # Headers: module headers keep their d4np/ prefix (include/d4np/<module>/*.h); the umbrella
    # header lands at the include root so consumers write `#include "d4np_c.h"`.
    install(DIRECTORY d4np/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/d4np
        FILES_MATCHING PATTERN "*.h" PATTERN "d4np_c.h" EXCLUDE)
    install(FILES d4np/d4np_c.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    # Exported target file: consumers link `d4np::d4np`.
    install(EXPORT d4np-cTargets
        FILE d4np-cTargets.cmake
        NAMESPACE d4np::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/d4np-c)

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/d4np-cConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/d4np-cConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/d4np-c)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/d4np-cConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/d4np-cConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/d4np-cConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/d4np-c)
endif()

# ---------------------------------------------------------------------------
# Tests (Unity, fetched only when D4NP_BUILD_TESTS is ON)
# ---------------------------------------------------------------------------
if(D4NP_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG v2.6.0)
    FetchContent_MakeAvailable(unity)
    # Enable Unity's double-precision assertions (off by default); PUBLIC so the test target
    # that links unity also sees the macro when including unity.h.
    target_compile_definitions(unity PUBLIC UNITY_INCLUDE_DOUBLE)

    add_executable(d4np_tests
        tests/test_main.c
        tests/str/test_str_view.c
        tests/str/test_parse.c
        tests/mem/test_arena.c
        tests/mem/test_pool.c
        tests/ds/test_vector.c
        tests/ds/test_string_builder.c
        tests/ds/test_hashmap.c
        tests/ds/test_linked_list.c
        tests/ds/test_ring_buffer.c
        tests/concurrency/test_mutex.c
        tests/concurrency/test_semaphore.c
        tests/concurrency/test_atomic_queue.c
        tests/concurrency/test_thread_pool.c
        tests/io/test_file.c
        tests/io/test_path.c
        tests/sys/test_hash.c
        tests/sys/test_clock.c
        tests/sys/test_uuid.c
        tests/sys/test_error_context.c
        tests/sys/test_log.c)
    target_include_directories(d4np_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(d4np_tests PRIVATE d4np::d4np unity)

    add_test(NAME d4np_tests COMMAND d4np_tests)
endif()

# ---------------------------------------------------------------------------
# Benchmarks (added per module as they land; gated by D4NP_BUILD_BENCH)
# ---------------------------------------------------------------------------
if(D4NP_BUILD_BENCH)
    add_executable(d4np_bench_mem bench/mem/bench_mem.c)
    target_link_libraries(d4np_bench_mem PRIVATE d4np::d4np)

    add_executable(d4np_bench_ds bench/ds/bench_ds.c)
    target_link_libraries(d4np_bench_ds PRIVATE d4np::d4np)

    add_executable(d4np_bench_concurrency bench/concurrency/bench_concurrency.c)
    target_link_libraries(d4np_bench_concurrency PRIVATE d4np::d4np)
endif()
