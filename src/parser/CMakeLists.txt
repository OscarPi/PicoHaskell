add_library(parser INTERFACE)
target_include_directories(parser INTERFACE include)
target_sources(parser INTERFACE driver.cpp)
