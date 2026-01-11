cmake_minimum_required(VERSION 3.20)
project(hft_elements VERSION 0.1.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# AMD optimization flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Build all static libraries
add_subdirectory(src/common)
add_subdirectory(src/dbn)
add_subdirectory(src/mbo_mbl)
add_subdirectory(src/snapshot)
add_subdirectory(src/network)
add_subdirectory(src/metrics)

# Build executables
add_subdirectory(src/simulator)
add_subdirectory(src/server)
add_subdirectory(src/collector)

# Tests (optional, enable with -DBUILD_TESTS=ON)
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

