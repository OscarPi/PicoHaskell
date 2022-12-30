add_executable(types_test types_test.cpp)
target_link_libraries(types_test test_utilities PicoHaskell GTest::gtest_main)
gtest_discover_tests(types_test)
