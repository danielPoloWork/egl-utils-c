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
    d4np/str/str_view.c)
add_library(d4np::d4np ALIAS d4np)

target_include_directories(d4np
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/d4np>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(d4np PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(d4np PRIVATE /W4 /permissive-)
else()
    target_compile_options(d4np PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wstrict-prototypes)
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

    add_executable(d4np_tests
        tests/str/test_str_view.c)
    target_link_libraries(d4np_tests PRIVATE d4np::d4np unity)

    add_test(NAME d4np_tests COMMAND d4np_tests)
endif()

# ---------------------------------------------------------------------------
# Benchmarks (added per module as they land; gated by D4NP_BUILD_BENCH)
# ---------------------------------------------------------------------------
if(D4NP_BUILD_BENCH)
    # Benchmark targets are added here as the perf-critical modules (mem, ds, concurrency, str)
    # land. See docs/benchmarks/ for the methodology.
endif()
