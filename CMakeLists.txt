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
    d4np/sys/uuid.c)
add_library(d4np::d4np ALIAS d4np)

target_include_directories(d4np
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/d4np>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(d4np PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(d4np PRIVATE /W4 /permissive-)
    # MSVC gates C11 <stdatomic.h> behind this switch; PUBLIC so consumers that include the
    # ring_buffer header (which uses C11 atomics) also enable it. Drop once MSVC un-gates it.
    target_compile_options(d4np PUBLIC /experimental:c11atomics)
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
        tests/sys/test_uuid.c)
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
