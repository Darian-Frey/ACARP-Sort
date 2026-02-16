cmake_minimum_required(VERSION 3.10)
project(ACARP_Sort)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

# Header-only library
add_library(acarp_sort INTERFACE)
target_include_directories(acarp_sort INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Benchmark executable
add_executable(bench benchmark_acarp.cpp)
target_link_libraries(bench PRIVATE acarp_sort)
