cmake_minimum_required(VERSION 3.5)

project(cache-allocator VERSION 1.0.0)

include_directories(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src)

enable_testing()
add_subdirectory(test)
