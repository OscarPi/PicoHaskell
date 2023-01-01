add_library(prelude INTERFACE)
target_include_directories(prelude INTERFACE include)
target_sources(prelude INTERFACE prelude.cpp)
