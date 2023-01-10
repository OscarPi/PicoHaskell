add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(types)
add_subdirectory(stg)
add_subdirectory(prelude)
add_subdirectory(generation)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(Parser parser/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.hpp)
FLEX_TARGET(Lexer lexer/lex.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer/lexer.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lexer/lexer.hpp)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

target_sources(lexer INTERFACE ${FLEX_Lexer_OUTPUTS})
target_include_directories(lexer INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(lexer INTERFACE ${FLEX_LIBRARIES})
target_sources(parser INTERFACE ${BISON_Parser_OUTPUTS})
target_include_directories(parser INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(picohaskell main.cpp)
target_link_libraries(picohaskell lexer parser types stg prelude generation)

add_library(PicoHaskell INTERFACE)
target_link_libraries(PicoHaskell INTERFACE lexer parser types stg prelude generation)
