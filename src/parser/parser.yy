%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.1"
%header
%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires {
    #include <string>
    #include <memory>
    #include "parser/syntax.hpp"
    class Driver;
}
%param { Driver& drv }
%parse-param { std::shared_ptr<SyntaxTreeNode> *result }
%locations
%define parse.trace
%define parse.error detailed
%define parse.lac full
%code {
    #include "lexer/layout.hpp"
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

%nterm <std::shared_ptr<SyntaxTreeNode>> exp;

//%printer { yyo << $$; } <*>;

%%
%start exp;

exp: "\\" { $$ = std::make_shared<Literal>(); *result = $$; };
//apatlist: apat | apatlist apat;
//lexp:
//    "\" apatlist "->" exp
//  | "let" decls "in" exp
//  | "if" exp optsemicolon "then" exp optsemicolon "else" exp
//  | "case" exp "of" "{" alts "}"
//  | "do" "{" stmts "}"
//  | fexp

//fexp: aexp | fexp aexp;

//fbindlist: fbind | fbindlist "," fbind;
//optfbindlist: %empty | fbindlist;
//quallist: qual | quallist "," qual;
//explist: exp | explist "," exp;
//optexp: %empty | exp;
//optexpcomma: %empty | "," exp;
//aexp:
//    qvar
//  | gcon
//  | literal
//  | "(" exp ")"
//  | "(" exp "," explist ")"
//  | "[" explist "]"
//  | "[" exp optexpcomma ".." optexp "]"
//  | "[" exp "|" quallist "]"
//  | "(" infixexp qop ")"
//  | "(" qop infixexp ")" //TODO: -
//  | qcon "{" optfbindlist "}"
//  | aexp "{" fbindlist "}" //TODO: not qcon
//  ;
//
//qual:
//    pat "<-" exp
//  | "let" decls
//  | exp
//  ;
//
//alts: alt | alts ";" alt;
//optwhere: %empty | "where" decls;
//alt:
//  | pat "->" exp optdecls
//  | pat gdpat optdecls
//  | %empty
//  ;
//
//gdpat: guards "->" exp | guards "->" exp gdpat;
//
//stmtlist: stmt | stmtlist "," stmt;
//optstmtlist: %empty | stmtlist;
//optsemicolon: %empty | ";";
//stmts: optstmtlist exp optsemicolon;
//stmt:
//    exp ";"
//  | pat "<-" exp ";"
//  | "let" decls ";"
//  | ";"
//  ;
//
//fbind: qvar "=" exp;
//
//pat:
//    lpat qconop pat
//  | lpat
//  ;
//
//apatlist: apat | apatlist apat;
//lpat:
//    apat
//  | "-" INTEGER
//  | "-" FLOAT
//  | gcon apatlist
//  ;
//
//optat: "@" apat | %empty;
//fpats: fpat | fpats "," fpat;
//fpatlist: %empty | fpats;
//optfpatlist: "{" fpatlist "}" | %empty;
//patlist: pat | patlist "," pat;
//apat:
//    qvar optat
//  | gcon
//  | qcon optfpatlist
//  | literal
//  | "_"
//  | "(" pat ")"
//  | "(" pat, patlist ")"
//  | "[" patlist "]"
//  | "~" apat
//  ;
//
//fpat: qvar "=" pat;
//
//gcon:
//    "(" ")"
//  | "[" "]"
//  | "(" "," "{" "," "}" ")"
//  | qcon
//  ;
//
//qvar: VARID | "(" VARSYM ")";
//qcon: CONID | "(" gconsym ")";
//qvarop: VARSYM | "`" VARID "`";
//qconop: gconsym | "`" CONID "`";
//qop: qvarop | qconop;
//gconsym: ":" | CONSYM;
//literal: INTEGER | FLOAT | CHAR | STRING;

%%
void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}
