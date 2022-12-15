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
    PLUS          "+"
    MINUS         "-"
    TIMES         "*"
    DIVIDE        "/"
    EQUALITY      "=="
    INEQUALITY    "/="
    LT            "<"
    LTE           "<="
    GT            ">"
    GTE           ">="
    AND           "&&"
    OR            "||"
    DOT           "."
;
%token <std::string> VARID CONID STRING
%token <int> INTEGER
%token <double> FLOAT
%token <char> CHAR

%right "||"
%right "&&"
%precedence "==" "/=" "<" "<=" ">=" ">"
%right ":"
%left "+" "-"
%left "*" "/"
%right "."

%nterm <std::string> var
%nterm <std::vector<std::string>> vars tyvars
%nterm <type> ctype btype atype gtycon
%nterm <std::vector<type>> types atypes
%nterm <int> commas
%nterm <std::pair<std::string, std::vector<std::string>>> simpletype
%nterm <std::vector<std::shared_ptr<DConstructor>>> constrs
%nterm <std::shared_ptr<DConstructor>> constr
%nterm <std::shared_ptr<Expression>> exp infixexp lexp fexp aexp gcon
%nterm <std::pair<std::string, std::shared_ptr<Expression>>> vardecl
%nterm <std::tuple<std::string, std::vector<std::string>, std::shared_ptr<Expression>>> fundecl
%nterm <std::pair<std::vector<std::string>, type>> typesig
%nterm <std::vector<std::shared_ptr<Expression>>> explist

//%printer { yyo << $$; } <*>;

%%
%start topdecls;

topdecls:
    topdecl
  | topdecls ";" topdecl
  ;

topdecl:
    typesig                       { program->addTypeSignatures($1.first, $1.second); }
  | fundecl                       { program->addNamedFunction(@1.begin.line, std::get<0>($1), std::get<1>($1), std::get<2>($1)); }
  | vardecl                       { program->addVariable(@1.begin.line, $1.first, $1.second); }
  | "data" simpletype             { program->addTypeConstructor(@1.begin.line, $2.first, $2.second, {}); }
  | "data" simpletype "=" constrs { program->addTypeConstructor(@1.begin.line, $2.first, $2.second, $4); }
  ;

typesig: vars "::" ctype    { $$ = std::make_pair($1, $3); };
fundecl: VARID vars "=" exp { $$ = std::make_tuple($1, $2, $4); };
vardecl: VARID "=" exp      { $$ =  std::make_pair($1, $3); };

exp:
    infixexp { $$ = $1; }
  ;

infixexp:
    infixexp "+" infixexp  { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::add); }
  | infixexp "-" infixexp  { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::subtract); }
  | "-" infixexp           { $$ = std::make_shared<BuiltInOp>(@1.begin.line, nullptr, $2, builtinop::negate); }
  | infixexp "*" infixexp  { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::times); }
  | infixexp "/" infixexp  { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::divide); }
  | infixexp "==" infixexp { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::equality); }
  | infixexp "/=" infixexp { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::inequality); }
  | infixexp "<" infixexp  { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::lt); }
  | infixexp "<=" infixexp { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::lte); }
  | infixexp ">" infixexp  { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::gt); }
  | infixexp ">=" infixexp { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::gte); }
  | infixexp "&&" infixexp { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::land); }
  | infixexp "||" infixexp { $$ = std::make_shared<BuiltInOp>(@2.begin.line, $1, $3, builtinop::lor); }
  | infixexp "." infixexp  { $$ = std::make_shared<Application>(@2.begin.line, std::make_shared<Application>(@2.begin.line, std::make_shared<Variable>(@2.begin.line, "."), $1), $3); }
  | infixexp ":" infixexp  { $$ = std::make_shared<Application>(@2.begin.line, std::make_shared<Application>(@2.begin.line, std::make_shared<Constructor>(@2.begin.line, ":"), $1), $3); }
  | lexp                   { $$ = $1; }
  ;

lexp:
    fexp                                                     { $$ = $1; }
  | "\\" vars "->" exp                                       { $$ = std::make_shared<Lambda>(@1.begin.line, $2, $4); }
  | "if" exp optsemicolon "then" exp optsemicolon "else" exp { $$ = makeIf(@1.begin.line, $2, $5, $8); }
  ;

fexp:
    fexp aexp { $$ = std::make_shared<Application>(@2.begin.line, $1, $2); }
  | aexp      { $$ = $1; }
  ;

aexp:
    var                     { $$ = std::make_shared<Variable>(@1.begin.line, $1); }
  | INTEGER                 { $$ = std::make_shared<Literal>(@1.begin.line, $1); }
  | STRING                  { $$ = std::make_shared<Literal>(@1.begin.line, $1); }
  | CHAR                    { $$ = std::make_shared<Literal>(@1.begin.line, $1); }
  | gcon                    { $$ = $1; }
  | "[" explist "]"         { $$ = makeList(@1.begin.line, $2); }
  | "(" explist "," exp ")" { $2.push_back($4); $$ = makeTuple(@1.begin.line, $2); }
  | "(" exp ")"             { $$ = $2; }
  ;

explist:
    exp             { $$ = {$1}; }
  | explist "," exp { $$ = $1; $$.push_back($3); }

vars:
    VARID      { $$ = {$1}; }
  | vars VARID { $$ = $1; $$.push_back($2); }
  ;

constrs:
    constr             { $$ = {$1}; }
  | constrs "|" constr { $$ = $1; $$.push_back($3); }

constr:
    CONID        { $$ = std::make_shared<DConstructor>(@1.begin.line, $1, std::vector<type>()); }
  | CONID atypes { $$ = std::make_shared<DConstructor>(@1.begin.line, $1, $2); }

atypes:
    atype        { $$ = {$1}; }
  | atypes atype { $$ = $1; $$.push_back($2); }

simpletype:
    CONID tyvars { $$ = std::make_pair($1, $2); }
  | CONID        { $$ = std::make_pair($1, std::vector<std::string>()); }
  ;

tyvars:
    VARID        { $$ = {$1}; }
  | tyvars VARID { $$ = $1; $$.push_back($2); }
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


gcon:
    "(" ")"        { $$ = std::make_shared<Constructor>(@1.begin.line, "()"); }
  | "[" "]"        { $$ = std::make_shared<Constructor>(@1.begin.line, "[]"); }
  | "(" commas ")" { $$ = std::make_shared<Constructor>(@1.begin.line, "(" + std::string($2, ',') + ")"); }
  | "(" ":" ")"    { $$ = std::make_shared<Constructor>(@1.begin.line, ":"); }
  | CONID          { $$ = std::make_shared<Constructor>(@1.begin.line, $1); }
  ;

  ;

//apatlist: apat | apatlist apat;

//fbindlist: fbind | fbindlist "," fbind;
//optfbindlist: %empty | fbindlist;
//quallist: qual | quallist "," qual;
//explist: exp | explist "," exp;
//optexp: %empty | exp;
//optexpcomma: %empty | "," exp;

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

var:
    VARID        { $$ = $1; }
  | "(" "." ")"  { $$ = "."; }
  | "(" "+" ")"  { $$ = "+"; }
  | "(" "-" ")"  { $$ = "-"; }
  | "(" "*" ")"  { $$ = "*"; }
  | "(" "/" ")"  { $$ = "/"; }
  | "(" "==" ")" { $$ = "=="; }
  | "(" "/=" ")" { $$ = "/="; }
  | "(" "<" ")"  { $$ = "<"; }
  | "(" "<=" ")" { $$ = "<="; }
  | "(" ">" ")"  { $$ = ">"; }
  | "(" ">=" ")" { $$ = ">="; }
  | "(" "&&" ")" { $$ = "&&"; }
  | "(" "||" ")" { $$ = "||"; }

optsemicolon: %empty | ";";

%%
void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}
