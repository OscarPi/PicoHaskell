#ifndef PICOHASKELL_TEST_UTILITIES_HPP
#define PICOHASKELL_TEST_UTILITIES_HPP

#include "parser/parser.hpp"

std::vector<yy::parser::symbol_type> lex_string(const char* str);
int parse_string(const char* str, Program *program);

bool same_type(const Type *a, const Type *b);

#endif //PICOHASKELL_TEST_UTILITIES_HPP
