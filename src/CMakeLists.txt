cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

set(TARGET_NAME cache-allocator-lib)

set(SOURCES
    cache.h
    cache.cpp
    list.h
    list.cpp
    memory.h
    memory.cpp
)

add_library(${TARGET_NAME} STATIC ${HEADERS} ${SOURCES})
