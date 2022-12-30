add_executable(lexer_test lexer_test.cpp)
target_link_libraries(lexer_test test_utilities PicoHaskell GTest::gtest_main)
gtest_discover_tests(lexer_test)
