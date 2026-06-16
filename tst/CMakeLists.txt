add_library(test_support STATIC
    main_tst.cpp
)

target_include_directories(test_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test_support PUBLIC
    GTest::gtest
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    maye_sql_lib)

add_maye_test(disk_test           storage/disk_manager/disk_manager_test.cpp)
add_maye_test(lru_replacer_test   storage/lru_replacer/lru_replacer_test.cpp)
add_maye_test(lruk_replacer_test  storage/lru_replacer/lruk_replacer_test.cpp)
add_maye_test(disk_scheduler_test storage/disk_scheduler/disk_scheduler_test.cpp)
add_maye_test(buffer_pool_test    storage/buffer_pool/buffer_pool_test.cpp)
add_maye_test(table_page_test     storage/table_page/table_page_test.cpp)
add_maye_test(table_heap_test     storage/table_heap/table_heap_test.cpp)
add_maye_test(catalog_test     storage/catalog/catalog_test.cpp)
add_maye_test(value_test       type/value_test.cpp)
