add_library(prelude INTERFACE)
target_include_directories(prelude INTERFACE include)
target_link_libraries(prelude INTERFACE lexer parser)
target_sources(prelude INTERFACE prelude.cpp)
