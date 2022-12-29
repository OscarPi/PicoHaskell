add_executable(stg_test stg_test.cpp)
target_link_libraries(stg_test PicoHaskell GTest::gtest_main)
gtest_discover_tests(stg_test)
