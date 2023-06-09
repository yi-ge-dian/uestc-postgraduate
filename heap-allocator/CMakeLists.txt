cmake_minimum_required(VERSION 3.16)
project(heap_allocator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# 头文件
include_directories(
        ${PROJECT_SOURCE_DIR}/src/heap-allocator
        ${PROJECT_SOURCE_DIR}/src/thread-cache
        ${PROJECT_SOURCE_DIR}/src/central-cache
        ${PROJECT_SOURCE_DIR}/src/page-cache
        ${PROJECT_SOURCE_DIR}/src/util
)

# 源文件
file(GLOB_RECURSE heap_allocator_source
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/util/*.cpp
        ${PROJECT_SOURCE_DIR}/src/heap-allocator/*.cpp
        ${PROJECT_SOURCE_DIR}/src/thread-cache/*.cpp
        ${PROJECT_SOURCE_DIR}/src/central-cache/*.cpp
        ${PROJECT_SOURCE_DIR}/src/page-cache/*.cpp
        )


# 执行文件
add_executable(heap_allocator ${heap_allocator_source} src/util/util.cpp)
