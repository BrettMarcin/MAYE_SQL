add_library(maye_sql_lib
    storage/disk_manager.cpp
    storage/lru_replacer.cpp
    storage/lruk_replacer.cpp
    storage/disk_scheduler.cpp
    storage/buffer_pool_manager.cpp
    storage/table_page.cpp
    storage/table_heap.cpp
)

target_include_directories(maye_sql_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(maye_sql_lib PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog)
target_compile_features(maye_sql_lib PUBLIC cxx_std_17)
