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


include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.x
)

FetchContent_MakeAvailable(spdlog)

add_library(maye_sql_lib 
    src/storage/disk_manager.cpp
    src/storage/lru_replacer.cpp
    src/storage/lruk_replacer.cpp
    src/storage/disk_scheduler.cpp
    src/storage/buffer_pool_manager.cpp
)

add_library(test_support STATIC 
    tst/main_tst.cpp
)

target_include_directories(maye_sql_lib PUBLIC include)
target_link_libraries(maye_sql_lib PRIVATE 
    nlohmann_json::nlohmann_json 
    spdlog::spdlog)
target_compile_features(maye_sql_lib PUBLIC cxx_std_17)

add_executable(maye_sql src/main.cpp)
target_link_libraries(maye_sql PRIVATE
    maye_sql_lib 
    nlohmann_json::nlohmann_json
    spdlog::spdlog)

target_include_directories(test_support PUBLIC tst/include)
target_link_libraries(test_support PUBLIC 
    GTest::gtest
    spdlog::spdlog 
    nlohmann_json::nlohmann_json 
    maye_sql_lib)

# 2: TEST EXECUTABLES
# 2.1: disk_test
add_executable(disk_test tst/storage/disk_manager/disk_manager_test.cpp)
target_link_libraries(disk_test PRIVATE 
    test_support
)

# 2.2: lru_replacer_test
add_executable(lru_replacer_test tst/storage/lru_replacer/lru_replacer_test.cpp)
target_link_libraries(lru_replacer_test PRIVATE 
    test_support
)

# 2.3: lruk_replacer_test
add_executable(lruk_replacer_test tst/storage/lru_replacer/lruk_replacer_test.cpp)
target_link_libraries(lruk_replacer_test PRIVATE 
    test_support
)

# 2.34 disk_scheduler_test
add_executable(disk_scheduler_test tst/storage/disk_scheduler/disk_scheduler_test.cpp)
target_link_libraries(disk_scheduler_test PRIVATE 
    test_support
)

add_executable(buffer_pool_test tst/storage/buffer_pool/buffer_pool_test.cpp)
target_link_libraries(buffer_pool_test PRIVATE 
    test_support
)

enable_testing()
add_test(NAME LRUReplacerTest COMMAND lru_replacer_test)
add_test(NAME LRUKReplacerTest COMMAND lruk_replacer_test)
add_test(NAME DiskSchedulerTest COMMAND disk_scheduler_test)
add_test(NAME DiskManagerTest COMMAND disk_test)
add_test(NAME BufferPoolManagerTest COMMAND buffer_pool_test)