cmake_minimum_required(VERSION 3.20)
project(d4np_test_package C)

find_package(d4np-c CONFIG REQUIRED)

add_executable(example example.c)
target_link_libraries(example PRIVATE d4np::d4np)
