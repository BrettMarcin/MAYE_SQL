cmake_minimum_required(VERSION 3.10)
project(MAYE_SQL VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -Wpedantic)
endif()

set(SOURCE_FILES 
    src/main.cpp
    src/storage/disk_manager.cpp)

add_executable(maye_sql ${SOURCE_FILES})

target_link_libraries(maye_sql PRIVATE nlohmann_json::nlohmann_json)

target_include_directories(maye_sql PUBLIC include)