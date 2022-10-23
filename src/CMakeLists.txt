add_subdirectory(parser)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(Parser parser/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.hpp)
FLEX_TARGET(Lexer lexer/lex.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

add_executable(PicoHaskell main.cpp ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})
target_include_directories(PicoHaskell PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(PicoHaskell parser ${FLEX_LIBRARIES})
