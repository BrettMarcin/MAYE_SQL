cmake_minimum_required(VERSION 3.10)
project(MAYE_SQL VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -Wpedantic)
endif()

find_package(nlohmann_json REQUIRED)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

add_library(maye_sql_lib 
    src/storage/disk_manager.cpp
)

target_include_directories(maye_sql_lib PUBLIC include)
target_link_libraries(maye_sql_lib PRIVATE nlohmann_json::nlohmann_json)
target_compile_features(maye_sql_lib PUBLIC cxx_std_17)

add_executable(maye_sql src/main.cpp)
target_link_libraries(maye_sql PRIVATE maye_sql_lib nlohmann_json::nlohmann_json)

add_executable(disk_test tst/disk_manager_test.cpp)
target_link_libraries(disk_test PRIVATE 
    GTest::gtest_main 
    maye_sql_lib 
    nlohmann_json::nlohmann_json
)

enable_testing()
add_test(NAME DiskManagerTest COMMAND disk_test)