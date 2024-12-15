cmake_minimum_required(VERSION 3.31)

project(ParallelQuicksort)

set(CMAKE_CXX_STANDARD 17)

find_package(TBB REQUIRED)

add_executable(parallel_bfs bfs.cpp)

target_link_libraries(parallel_bfs TBB::tbb)