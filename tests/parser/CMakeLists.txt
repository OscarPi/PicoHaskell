add_executable(parser_test parser_test.cpp)
target_link_libraries(parser_test test_utilities PicoHaskell GTest::gtest_main)
gtest_discover_tests(parser_test)
