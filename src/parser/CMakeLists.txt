add_library(parser INTERFACE)
target_include_directories(parser INTERFACE include)
target_link_libraries(parser INTERFACE lexer types)
target_sources(parser INTERFACE syntax.cpp)
