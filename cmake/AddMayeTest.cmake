# Helper: define a test executable, link it against test_support, and
# auto-register every TEST() in it with CTest via gtest_discover_tests.
function(add_maye_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE test_support)
    gtest_discover_tests(${test_name})
endfunction()
