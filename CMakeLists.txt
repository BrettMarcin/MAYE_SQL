cmake_minimum_required(VERSION 3.10)
project(MAYE_SQL VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put all executables directly in build/ regardless of which subdirectory
# defines them, so binary paths stay stable (e.g. for launch.json).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Default to a Debug build (with -g) so breakpoints work out of the box.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -Wpedantic)
endif()

# 1. PACKAGE DEPENDENCIES
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)
include(GoogleTest)


include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.x
)

FetchContent_MakeAvailable(spdlog)

# Shared CMake helpers (e.g. add_maye_test)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(AddMayeTest)

# Library targets
add_subdirectory(src)

# Main executable
add_executable(maye_sql src/main.cpp)
target_link_libraries(maye_sql PRIVATE
    maye_sql_lib
    nlohmann_json::nlohmann_json
    spdlog::spdlog)

# Tests
enable_testing()
add_subdirectory(tst)