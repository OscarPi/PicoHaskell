add_library(lexer INTERFACE)
target_include_directories(lexer INTERFACE include)
target_sources(lexer INTERFACE layout.cpp)
