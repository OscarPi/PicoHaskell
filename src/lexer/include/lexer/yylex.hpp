#ifndef PICOHASKELL_LEXER_HPP
#define PICOHASKELL_LEXER_HPP

#include "parser/parser.hpp"

#define YY_DECL yy::parser::symbol_type yylex(yy::location &loc)
YY_DECL;

#endif //PICOHASKELL_LEXER_HPP
