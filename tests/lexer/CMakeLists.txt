add_executable(lexer_test lexer_test.cpp)
target_link_libraries(lexer_test PicoHaskell GTest::gtest_main)
gtest_discover_tests(lexer_test)
