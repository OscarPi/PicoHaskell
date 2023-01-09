add_library(generation INTERFACE)
target_include_directories(generation INTERFACE include)
target_sources(generation INTERFACE generation.cpp)
