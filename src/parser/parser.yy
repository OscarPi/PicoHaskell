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
    #include "types/types.hpp"
    class Driver;
}
%param { Driver& drv }
%parse-param { std::shared_ptr<Program> program }
%locations
%define parse.trace
%define parse.error detailed
%define parse.lac full
%code {
    #include "parser/driver.hpp"
}
%define api.token.prefix {TOK_}
%token
    NEWLINE       "\n"
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
%token <std::string> VARID CONID VARSYM CONSYM STRING
%token <int> INTEGER
%token <double> FLOAT
%token <char> CHAR

%nterm <std::vector<std::string>> vars
%nterm <std::string> var
%nterm <type> ctype btype atype gtycon
%nterm <std::vector<type>> types
%nterm <int> commas

//%printer { yyo << $$; } <*>;

%%
%start prog;

prog: topdecls | topdecls break;

topdecls:
//    typedecl
    decl
//  | topdecls typedecl
  | topdecls break decl
  ;

decl:
    gendecl
//  | funlhs rhs
//  | pat rhs
  ;

gendecl:
    vars "::" ctype { program->addTypeSignatures($1, $3); }
  ;

vars:
    var      { $$ = {$1}; }
  | vars var { $$ = $1; $$.push_back($2); }
  ;

ctype:
    btype            { $$ = $1; }
  | btype "->" ctype { $$ = makeFunctionType($1, $3); }
  ;

btype:
    atype       { $$ = $1; }
  | btype atype { $$ = std::make_shared<const TypeApplication>($1, $2); }
  ;

atype:
   gtycon        { $$ = $1; }
 | VARID         { $$ = std::make_shared<const TypeVariable>($1, nullptr); }
 | "(" types ")" { $$ = makeTupleType($2); }
 | "[" ctype "]"  { $$ = makeListType($2); }
 | "(" ctype ")"  { $$ = $2; }
 ;

types:
    ctype "," ctype  { $$ = {$1, $3}; }
  | types "," ctype { $$ = $1; $$.push_back($3); }
  ;

gtycon:
    CONID          { $$ = std::make_shared<const TypeConstructor>($1, nullptr); }
  | "(" ")"        { $$ = tUnit; }
  | "[" "]"        { $$ = tList; }
  | "(" "->" ")"   { $$ = tArrow; }
  | "(" commas ")" { $$ = std::make_shared<const TypeConstructor>("(" + std::string($2, ',') + ")", makeTupleConstructorKind($2 + 1)); }
  ;

commas:
    ","        { $$ = 1; }
  | commas "," { $$ = $1 + 1; }
  ;

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

//gcon:
//    "(" ")"
//  | "[" "]"
//  | "(" "," "{" "," "}" ")"
//  | qcon
//  ;

var:
    VARID          { $$ = $1; }
  | "(" VARSYM ")" { $$ = $2; }
  ;
//qcon: CONID | "(" gconsym ")";
//qvarop: VARSYM | "`" VARID "`";
//qconop: gconsym | "`" CONID "`";
//qop: qvarop | qconop;
//gconsym: ":" | CONSYM;
//literal:
//    INTEGER { $$ = std::make_shared<Literal<int>>($1); }
//  | FLOAT   { $$ = std::make_shared<Literal<double>>($1); }
//  | CHAR    { $$ = std::make_shared<Literal<char>>($1); }
//  | STRING  { $$ = std::make_shared<Literal<std::string>>($1); }
//  ;

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

break: NEWLINE | break NEWLINE;
optbreak: %empty | break;

%%
void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}
