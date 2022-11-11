#include "lexer/layout.hpp"

yy::parser::symbol_type yylex(Driver& drv) {
    return yyflex(drv);
}
