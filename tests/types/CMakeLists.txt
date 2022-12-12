add_executable(types_test types_test.cpp)
target_link_libraries(types_test PicoHaskell GTest::gtest_main)
gtest_discover_tests(types_test)
