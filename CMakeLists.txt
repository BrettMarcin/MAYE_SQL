cmake_minimum_required(VERSION 3.10)
project(MAYE_SQL VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
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

add_library(maye_sql_lib 
    src/storage/disk_manager.cpp
    src/storage/lru_replacer.cpp
)

target_include_directories(maye_sql_lib PUBLIC include)
target_link_libraries(maye_sql_lib PRIVATE nlohmann_json::nlohmann_json)
target_compile_features(maye_sql_lib PUBLIC cxx_std_17)

add_executable(maye_sql src/main.cpp)
target_link_libraries(maye_sql PRIVATE maye_sql_lib nlohmann_json::nlohmann_json)

# 2: TEST EXECUTABLES
# 2.1: disk_test
add_executable(disk_test tst/storage/disk_manager/disk_manager_test.cpp)
target_link_libraries(disk_test PRIVATE 
    GTest::gtest_main 
    maye_sql_lib 
    nlohmann_json::nlohmann_json
)

# 2.2: lru_replacer_test
add_executable(lru_replacer_test tst/storage/disk_manager/lru_replacer_test.cpp)
target_link_libraries(lru_replacer_test PRIVATE 
    GTest::gtest_main 
    maye_sql_lib 
    nlohmann_json::nlohmann_json
)

enable_testing()
add_test(NAME LRUReplacerTest COMMAND lru_replacer_test)
add_test(NAME DiskManagerTest COMMAND disk_test)