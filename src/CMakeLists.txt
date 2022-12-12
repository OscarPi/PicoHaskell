add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(types)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(Parser parser/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.hpp)
FLEX_TARGET(Lexer lexer/lex.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer/lexer.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lexer/lexer.hpp)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

add_executable(main main.cpp ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})
target_include_directories(main PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(main lexer parser types ${FLEX_LIBRARIES})

add_library(PicoHaskell INTERFACE)
target_include_directories(PicoHaskell INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_sources(PicoHaskell INTERFACE ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})
target_link_libraries(PicoHaskell INTERFACE lexer parser types ${FLEX_LIBRARIES})
