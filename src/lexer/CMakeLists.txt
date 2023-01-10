add_library(lexer INTERFACE)
target_include_directories(lexer INTERFACE include)
target_link_libraries(lexer INTERFACE parser)
