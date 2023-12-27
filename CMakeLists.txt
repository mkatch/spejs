cmake_minimum_required(VERSION 3.28.1 FATAL_ERROR)
project(Spejs)

set(CMAKE_DISABLE_SOURCE_CHANGES OFF)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 20)

find_package(gRPC REQUIRED)

add_subdirectory(src/proto)
add_subdirectory(src/universe)
add_subdirectory(src/frontend)