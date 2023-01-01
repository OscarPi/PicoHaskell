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
}
%param { yy::location &loc }
%parse-param { Program *program }
%locations
%define parse.trace
%define parse.error detailed
%define parse.lac full
%code {
    #include "lexer/yylex.hpp"
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

%nterm <std::string> var gcon
%nterm <std::vector<std::string>> vars tyvars
%nterm <Type*> ctype btype atype gtycon
%nterm <std::vector<Type*>> types atypes
%nterm <int> commas
%nterm <std::pair<std::string, std::vector<std::string>>> simpletype
%nterm <std::vector<DConstructor*>> constrs
%nterm <DConstructor*> constr
%nterm <Expression*> exp infixexp lexp fexp aexp
%nterm <std::pair<std::string, Expression*>> vardecl
%nterm <std::tuple<std::string, std::vector<std::string>, Expression*>> fundecl
%nterm <std::pair<std::string, Type*>> typesig
%nterm <std::vector<Expression*>> explist
%nterm <declist> decls
%nterm <Pattern*> pat lpat apat
%nterm <std::vector<Pattern*>> pats apats
%nterm <std::pair<Pattern*, Expression*>> alt
%nterm <std::vector<std::pair<Pattern*, Expression*>>> alts

//%printer { yyo << $$; } <*>;

%%
%start topdecls;

topdecls:
    topdecl
  | topdecls ";" topdecl
  ;

topdecl:
    typesig                       { program->add_type_signature(@1.begin.line, $1.first, $1.second); }
  | fundecl                       { program->add_named_function(@1.begin.line, std::get<0>($1), std::get<1>($1), std::get<2>($1)); }
  | vardecl                       { program->add_variable(@1.begin.line, $1.first, $1.second); }
  | "data" simpletype             { program->add_type_constructor(@1.begin.line, $2.first, $2.second, {}); }
  | "data" simpletype "=" constrs { program->add_type_constructor(@1.begin.line, $2.first, $2.second, $4); }
  ;

decls:
    %empty            { $$ = std::make_tuple<typesigs, funcs, vars>({}, {}, {}); }
  | typesig           { $$ = std::make_tuple<typesigs, funcs, vars>({$1}, {}, {}); }
  | fundecl           { $$ = std::make_tuple<typesigs, funcs, vars>({}, {$1}, {}); }
  | vardecl           { $$ = std::make_tuple<typesigs, funcs, vars>({}, {}, {$1}); }
  | decls ";" typesig { $$ = $1; std::get<0>($$).push_back($3); }
  | decls ";" fundecl { $$ = $1; std::get<1>($$).push_back($3); }
  | decls ";" vardecl { $$ = $1; std::get<2>($$).push_back($3); }

typesig: var "::" ctype   { $$ = std::make_pair($1, $3); };
fundecl: var vars "=" exp { $$ = std::make_tuple($1, $2, $4); };
vardecl: var "=" exp      { $$ =  std::make_pair($1, $3); };

exp:
    infixexp { $$ = $1; }
  ;

infixexp:
    infixexp "+" infixexp  { $$ = new BuiltInOp(@2.begin.line, $1, $3, builtinop::add); }
  | infixexp "-" infixexp  { $$ = new BuiltInOp(@2.begin.line, $1, $3, builtinop::subtract); }
  | "-" infixexp           { $$ = new BuiltInOp(@1.begin.line, nullptr, $2, builtinop::negate); }
  | infixexp "*" infixexp  { $$ = new BuiltInOp(@2.begin.line, $1, $3, builtinop::times); }
  | infixexp "/" infixexp  { $$ = new BuiltInOp(@2.begin.line, $1, $3, builtinop::divide); }
  | infixexp "==" infixexp { $$ = new BuiltInOp(@2.begin.line, $1, $3, builtinop::equality); }
  | infixexp "/=" infixexp { $$ = new BuiltInOp(@2.begin.line, $1, $3, builtinop::inequality); }
  | infixexp "<" infixexp  { $$ = new BuiltInOp(@2.begin.line, $1, $3, builtinop::lt); }
  | infixexp "<=" infixexp { $$ = new BuiltInOp(@2.begin.line, $1, $3, builtinop::lte); }
  | infixexp ">" infixexp  { $$ = new BuiltInOp(@2.begin.line, $1, $3, builtinop::gt); }
  | infixexp ">=" infixexp { $$ = new BuiltInOp(@2.begin.line, $1, $3, builtinop::gte); }
  | infixexp "&&" infixexp { $$ = new Application(@2.begin.line, new Application(@2.begin.line, new Variable(@2.begin.line, "&&"), $1), $3); }
  | infixexp "||" infixexp { $$ = new Application(@2.begin.line, new Application(@2.begin.line, new Variable(@2.begin.line, "||"), $1), $3); }
  | infixexp "." infixexp  { $$ = new Application(@2.begin.line, new Application(@2.begin.line, new Variable(@2.begin.line, "."), $1), $3); }
  | infixexp ":" infixexp  { $$ = new Application(@2.begin.line, new Application(@2.begin.line, new Constructor(@2.begin.line, ":"), $1), $3); }
  | lexp                   { $$ = $1; }
  ;

lexp:
    fexp                                                     { $$ = $1; }
  | "\\" vars "->" exp                                       { $$ = new Abstraction(@1.begin.line, $2, $4); }
  | "if" exp optsemicolon "then" exp optsemicolon "else" exp { $$ = make_if_expression(@1.begin.line, $2, $5, $8); }
  | "let" "{" decls "}" "in" exp                             { $$ = make_let_expression(@1.begin.line, $3, $6); }
  | "case" exp "of" "{" alts "}"                             { $$ = new Case(@1.begin.line, $2, $5); }
  ;

fexp:
    fexp aexp { $$ = new Application(@2.begin.line, $1, $2); }
  | aexp      { $$ = $1; }
  ;

aexp:
    var                     { $$ = new Variable(@1.begin.line, $1); }
  | INTEGER                 { $$ = new Literal(@1.begin.line, $1); }
  | STRING                  { $$ = new Literal(@1.begin.line, $1); }
  | CHAR                    { $$ = new Literal(@1.begin.line, $1); }
  | gcon                    { $$ = new Constructor(@1.begin.line, $1); }
  | "[" explist "]"         { $$ = make_list_expression(@1.begin.line, $2); }
  | "(" explist "," exp ")" { $2.push_back($4); $$ = make_tuple_expression(@1.begin.line, $2); }
  | "(" exp ")"             { $$ = $2; }
  ;

explist:
    exp             { $$ = {$1}; }
  | explist "," exp { $$ = $1; $$.push_back($3); }

vars:
    var      { $$ = {$1}; }
  | vars var { $$ = $1; $$.push_back($2); }
  ;

constrs:
    constr             { $$ = {$1}; }
  | constrs "|" constr { $$ = $1; $$.push_back($3); }

constr:
    CONID        { $$ = new DConstructor(@1.begin.line, $1, {}); }
  | CONID atypes { $$ = new DConstructor(@1.begin.line, $1, $2); }

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
  | btype "->" ctype { $$ = make_function_type($1, $3); }
  ;

btype:
    atype       { $$ = $1; }
  | btype atype { $$ = new TypeApplication($1, $2); }
  ;

atype:
   gtycon        { $$ = $1; }
 | VARID         { $$ = new UniversallyQuantifiedVariable($1); }
 | "(" types ")" { $$ = make_tuple_type($2); }
 | "[" ctype "]" { $$ = make_list_type($2); }
 | "(" ctype ")" { $$ = $2; }
 ;

types:
    ctype "," ctype  { $$ = {$1, $3}; }
  | types "," ctype  { $$ = $1; $$.push_back($3); }
  ;

gtycon:
    CONID          { $$ = new TypeConstructor($1); }
  | "(" ")"        { $$ = new TypeConstructor("()"); }
  | "[" "]"        { $$ = new TypeConstructor("[]"); }
  | "(" "->" ")"   { $$ = new TypeConstructor("->"); }
  | "(" commas ")" { $$ = new TypeConstructor("(" + std::string($2, ',') + ")"); }
  ;

commas:
    ","        { $$ = 1; }
  | commas "," { $$ = $1 + 1; }
  ;


gcon:
    "(" ")"        { $$ = "()"; }
  | "[" "]"        { $$ = "[]"; }
  | "(" commas ")" { $$ = "(" + std::string($2, ',') + ")"; }
  | "(" ":" ")"    { $$ = ":"; }
  | CONID          { $$ = $1; }
  ;

alts:
    alt          { $$ = {$1}; }
  | alts ";" alt { $$ = $1; $$.push_back($3); }
  ;

alt: pat "->" exp { $$ = std::make_pair($1, $3); };

pat:
    lpat ":" pat { $$ = new ConstructorPattern(@1.begin.line, ":", std::vector<Pattern*>{$1, $3}); }
  | lpat         { $$ = $1; }
  ;

lpat:
    apat        { $$ = $1; }
  | "-" INTEGER { $$ = new LiteralPattern(@1.begin.line, -$2); }
  | gcon apats  { $$ = new ConstructorPattern(@1.begin.line, $1, $2); }
  ;

apat:
    var "@" apat       { $$ = $3; $$->as.push_back($1); }
  | var                { $$ = new VariablePattern(@1.begin.line, $1); }
  | gcon                 { $$ = new ConstructorPattern(@1.begin.line, $1, std::vector<Pattern*>{}); }
  | INTEGER              { $$ = new LiteralPattern(@1.begin.line, $1); }
  | STRING               { $$ = new LiteralPattern(@1.begin.line, $1); }
  | CHAR                 { $$ = new LiteralPattern(@1.begin.line, $1); }
  | "_"                  { $$ = new WildPattern(@1.begin.line); }
  | "(" pat ")"          { $$ = $2; }
  | "(" pats "," pat ")" { $2.push_back($4); $$ = make_tuple_pattern(@1.begin.line, $2); }
  | "[" pats "]"         { $$ = make_list_pattern(@1.begin.line, $2); }
  ;

apats:
    apat       { $$ = {$1}; }
  | apats apat { $$ = $1; $$.push_back($2); }
  ;

pats:
    pat          { $$ = {$1}; }
  | pats "," pat { $$ = $1; $$.push_back($3); }
  ;

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
  ;

optsemicolon: %empty | ";";

%%
void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}
