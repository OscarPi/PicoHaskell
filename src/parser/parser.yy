%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.1"
%header
%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires {
    #include <string>
    class Driver;
}
%param { Driver& drv }
%locations
%define parse.trace
%define parse.error detailed
%define parse.lac full
%code {
    #include "parser/driver.hpp"
}
%define api.token.prefix {TOK_}
%token
    CASE          "case"
    CLASS         "class"
    DATA          "data"
    DEFAULT       "default"
    DERIVING      "deriving"
    DO            "do"
    ELSE          "else"
    FOREIGN       "foreign"
    IF            "if"
    IMPORT        "import"
    IN            "in"
    INFIX         "infix"
    INFIXL        "infixl"
    INFIXR        "infixr"
    INSTANCE      "instance"
    LET           "let"
    MODULE        "module"
    NEWTYPE       "newtype"
    OF            "of"
    THEN          "then"
    TYPE          "type"
    WHERE         "where"
    _             "_"
    DOTDOT        ".."
    COLON         ":"
    HASTYPE       "::"
    EQUALS        "="
    BACKSLASH     "\\"
    PIPE          "|"
    LEFTARROW     "<-"
    RIGHTARROW    "->"
    AT            "@"
    TILDE         "~"
    IMPLIES       "=>"
    LEFTBRACKET   "("
    RIGHTBRACKET  ")"
    COMMA         ","
    SEMICOLON     ";"
    LEFTCROTCHET  "["
    RIGHTCROTCHET "]"
    BACKTICK      "`"
    LEFTBRACE     "{"
    RIGHTBRACE    "}"
;
%token <std::string> VARID
%token <std::string> CONID
%token <std::string> VARSYM
%token <std::string> CONSYM
%token <int> INTEGER
%token <double> FLOAT
%token <char> CHAR
%token <std::string> STRING
%nterm <int> exp
%printer { yyo << $$; } <*>;

%%
%start unit;
unit: COMMA {};

%%
void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}
