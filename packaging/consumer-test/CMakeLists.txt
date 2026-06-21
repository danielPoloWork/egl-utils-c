# Standalone consumer of an *installed* d4np-c, used by the CI `install` job to prove the
# exported CMake package (find_package(d4np-c) -> d4np::d4np) works end to end. Point it at the
# install prefix with -DCMAKE_PREFIX_PATH=<prefix>.
cmake_minimum_required(VERSION 3.20)
project(d4np_consumer_test C)

find_package(d4np-c CONFIG REQUIRED)

add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE d4np::d4np)

enable_testing()
add_test(NAME consumer COMMAND consumer)
