%{ /* -*- C++ -*- */
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>
#include "lexer/yylex.hpp"
#include "parser/parser.hpp"
%}
%option noyywrap nounput noinput batch debug stack
%{
yy::parser::symbol_type parse_integer(const std::string &s, const yy::parser::location_type &loc);
yy::parser::symbol_type parse_character(const std::string &s, const yy::parser::location_type &loc);
char parse_escape(const std::string &escape, const yy::parser::location_type &loc);
%}

%x incomment
%x instring
%x ingap

/* definitions */
LARGE       [A-Z]
SMALL       [a-z_]
DIGIT       [0-9]
OCTIT       [0-7]
HEXIT       [0-9A-Fa-f]
SYMBOL      [!#\$%&\*\+\.\/\<=\>\?@\\\^\|\-~\:]
SPECIAL     [\(\)\,\;\[\]\`\{\}]
GRAPHIC     {SMALL}|{LARGE}|{SYMBOL}|{DIGIT}|{SPECIAL}|\"|\'
ANY         {GRAPHIC}|" "|\t
NEWLINE     \r\n|\r|\n|\f

VARID       {SMALL}({SMALL}|{LARGE}|{DIGIT}|\')*
CONID       {LARGE}({SMALL}|{LARGE}|{DIGIT}|\')*

COMMENT     "--""-"*(({SMALL}|{LARGE}|{DIGIT}|{SPECIAL}|\"|\'|" "|\t){ANY}*)?{NEWLINE}
OPENCOM     "{""-"+
CLOSECOM    "-"+"}"
INCOM       {SMALL}|{LARGE}|{DIGIT}|[!#\$%&\*\+\.\/\<=\>\?@\\\^\|~\:\(\)\,\;\[\]\`\"\' \v]

DECIMAL     {DIGIT}+
OCTAL       {OCTIT}+
HEXADECIMAL {HEXIT}+
INTEGER     {DECIMAL}|"0"[oO]{OCTAL}|"0"[xX]{HEXADECIMAL}
EXPONENT    [eE][+-]?{DECIMAL}
FLOAT       {DECIMAL}"."{DECIMAL}{EXPONENT}?|{DECIMAL}{EXPONENT}

CHARESC     [abfnrtv\\\"\'&]
CNTRL       {LARGE}|[@\[\\\]\^_]
ASCII       "^"{CNTRL}|"NUL"|"SOH"|"STX"|"ETX"|"EOT"|"ENQ"|"ACK"|"BEL"|"BS"|"HT"|"LF"|"VT"|"FF"|"CR"|"SO"|"SI"|"DLE"|"DC1"|"DC2"|"DC3"|"DC4"|"NAK"|"SYN"|"ETB"|"CAN"|"EM"|"SUB"|"ESC"|"FS"|"GS"|"RS"|"US"|"SP"|"DEL"
ESCAPE      "\\"({CHARESC}|{ASCII}|{DECIMAL}|"o"{OCTAL}|"x"{HEXADECIMAL})
CHAR        \'(({SMALL}|{LARGE}|[!#\$%&\*\+\.\/\<=\>\?@\^\|\-~\:]|{DIGIT}|{SPECIAL}|\")|" "|{ESCAPE})\'
STRCHAR     {SMALL}|{LARGE}|[!#\$%&\*\+\.\/\<=\>\?@\^\|\-~\:]|{DIGIT}|{SPECIAL}|\'|" "
%{
#define YY_USER_ACTION  loc.columns(yyleng);
%}
%%
%{
loc.step();
std::string strliteral;
%}
[ \v]+      loc.step();
\t          loc.columns(((loc.end.column-1) % 8) == 0 ? 0 : 8-((loc.end.column-1) % 8)); loc.step();
{NEWLINE}   loc.lines(1); loc.step();
{COMMENT}   loc.lines(1); loc.step();
"case"      return yy::parser::make_CASE(loc);
"class"     return yy::parser::make_CLASS(loc);
"data"      return yy::parser::make_DATA(loc);
"default"   return yy::parser::make_DEFAULT(loc);
"deriving"  return yy::parser::make_DERIVING(loc);
"do"        return yy::parser::make_DO(loc);
"else"      return yy::parser::make_ELSE(loc);
"foreign"   return yy::parser::make_FOREIGN(loc);
"if"        return yy::parser::make_IF(loc);
"import"    return yy::parser::make_IMPORT(loc);
"in"        return yy::parser::make_IN(loc);
"infix"     return yy::parser::make_INFIX(loc);
"infixl"    return yy::parser::make_INFIXL(loc);
"infixr"    return yy::parser::make_INFIXR(loc);
"instance"  return yy::parser::make_INSTANCE(loc);
"let"       return yy::parser::make_LET(loc);
"module"    return yy::parser::make_MODULE(loc);
"newtype"   return yy::parser::make_NEWTYPE(loc);
"of"        return yy::parser::make_OF(loc);
"then"      return yy::parser::make_THEN(loc);
"type"      return yy::parser::make_TYPE(loc);
"where"     return yy::parser::make_WHERE(loc);
"_"         return yy::parser::make__(loc);
".."        return yy::parser::make_DOTDOT(loc);
":"         return yy::parser::make_COLON(loc);
"::"        return yy::parser::make_HASTYPE(loc);
"="         return yy::parser::make_EQUALS(loc);
"\\"        return yy::parser::make_BACKSLASH(loc);
"|"         return yy::parser::make_PIPE(loc);
"<-"        return yy::parser::make_LEFTARROW(loc);
"->"        return yy::parser::make_RIGHTARROW(loc);
"@"         return yy::parser::make_AT(loc);
"~"         return yy::parser::make_TILDE(loc);
"=>"        return yy::parser::make_IMPLIES(loc);
"("         return yy::parser::make_LEFTBRACKET(loc);
")"         return yy::parser::make_RIGHTBRACKET(loc);
","         return yy::parser::make_COMMA(loc);
";"         return yy::parser::make_SEMICOLON(loc);
"["         return yy::parser::make_LEFTCROTCHET(loc);
"]"         return yy::parser::make_RIGHTCROTCHET(loc);
"`"         return yy::parser::make_BACKTICK(loc);
"{"         return yy::parser::make_LEFTBRACE(loc);
"}"         return yy::parser::make_RIGHTBRACE(loc);
"+"         return yy::parser::make_PLUS(loc);
"-"         return yy::parser::make_MINUS(loc);
"*"         return yy::parser::make_TIMES(loc);
"/"         return yy::parser::make_DIVIDE(loc);
"=="        return yy::parser::make_EQUALITY(loc);
"/="        return yy::parser::make_INEQUALITY(loc);
"<"         return yy::parser::make_LT(loc);
"<="        return yy::parser::make_LTE(loc);
">"         return yy::parser::make_GT(loc);
">="        return yy::parser::make_GTE(loc);
"&&"        return yy::parser::make_AND(loc);
"||"        return yy::parser::make_OR(loc);
"."         return yy::parser::make_DOT(loc);
{VARID}     return yy::parser::make_VARID(yytext, loc);
{CONID}     return yy::parser::make_CONID(yytext, loc);
{OPENCOM}   yy_push_state(incomment);
<incomment>{
    {OPENCOM}           yy_push_state(incomment);
    {CLOSECOM}          yy_pop_state(); loc.step();
    {NEWLINE}           loc.lines(1);
    ({INCOM}|"}")*
    "-"+{INCOM}*
    "{"+({INCOM}|"}")*
    \t                  loc.columns(((loc.end.column-1) % 8) == 0 ? 0 : 8-((loc.end.column-1) % 8));
    .                   throw yy::parser::syntax_error(loc, "Invalid character: " + std::string(yytext));
}
{INTEGER}   return parse_integer(yytext, loc);
{FLOAT}     return yy::parser::make_FLOAT(std::stod(yytext), loc);
{CHAR}      return parse_character(yytext, loc);
\"          BEGIN(instring); strliteral.clear();
<instring>{
    \"             BEGIN(INITIAL); return yy::parser::make_STRING(strliteral, loc);
    \\[ \v]+       BEGIN(ingap);
    \\\t           loc.columns(((loc.end.column-1) % 8) == 0 ? 0 : 8-((loc.end.column-1) % 8)); BEGIN(ingap);
    \\{NEWLINE}    BEGIN(ingap); loc.lines(1);
    {ESCAPE}       if (strcmp(yytext, "\\&") != 0) { strliteral += parse_escape(yytext, loc); }
    {STRCHAR}+     strliteral += yytext;
    .              throw yy::parser::syntax_error(loc, "Invalid character in string literal: " + std::string(yytext));
}
<ingap>{
    [ \v]+
    \t           loc.columns(((loc.end.column-1) % 8) == 0 ? 0 : 8-((loc.end.column-1) % 8));
    {NEWLINE}    loc.lines(1);
    \\           BEGIN(instring);
    .            throw yy::parser::syntax_error(loc, "Invalid character in gap in string literal: " + std::string(yytext));
}
<<EOF>>     return yy::parser::make_YYEOF(loc);
.           throw yy::parser::syntax_error(loc, "Invalid character: " + std::string(yytext));
%%
yy::parser::symbol_type parse_integer(const std::string &s, const yy::parser::location_type& loc) {
    if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) {
        return yy::parser::make_INTEGER(std::stoi(s.substr(2), nullptr, 16), loc);
    } else if (s.rfind("0o", 0) == 0 || s.rfind("0O", 0) == 0) {
        return yy::parser::make_INTEGER(std::stoi(s.substr(2), nullptr, 8), loc);
    } else {
        return yy::parser::make_INTEGER(std::stoi(s, nullptr, 10), loc);
    }
}

char parse_escape(const std::string &escape, const yy::parser::location_type& loc) {
    int c = -1;

    if (isdigit(escape[1])) {
        c = std::stoi(escape.substr(1));
    } else if (escape[1] == 'o') {
        c = std::stoi(escape.substr(2), nullptr, 8);
    } else if (escape[1] == 'x') {
        c = std::stoi(escape.substr(2), nullptr, 16);
    } else if (escape == "\\a") {
        c = 0x07;
    } else if (escape == "\\b") {
        c = 0x08;
    } else if (escape == "\\f") {
        c = 0x0C;
    } else if (escape == "\\n") {
        c = 0x0A;
    } else if (escape == "\\r") {
        c = 0x0D;
    } else if (escape == "\\t") {
        c = 0x09;
    } else if (escape == "\\v") {
        c = 0x0B;
    } else if (escape == "\\\"") {
        c = 0x22;
    } else if (escape == "\\'") {
        c = 0x27;
    } else if (escape == "\\\\") {
        c = 0x5C;
    } else if (escape == "\\NUL") {
        c = 0x00;
    } else if (escape == "\\SOH") {
        c = 0x01;
    } else if (escape == "\\STX") {
        c = 0x02;
    } else if (escape == "\\ETX") {
        c = 0x03;
    } else if (escape == "\\EOT") {
        c = 0x04;
    } else if (escape == "\\ENQ") {
        c = 0x05;
    } else if (escape == "\\ACK") {
        c = 0x06;
    } else if (escape == "\\BEL") {
        c = 0x07;
    } else if (escape == "\\BS") {
        c = 0x08;
    } else if (escape == "\\HT") {
        c = 0x09;
    } else if (escape == "\\LF") {
        c = 0x0A;
    } else if (escape == "\\VT") {
        c = 0x0B;
    } else if (escape == "\\FF") {
        c = 0x0C;
    } else if (escape == "\\CR") {
        c = 0x0D;
    } else if (escape == "\\SO") {
        c = 0x0E;
    } else if (escape == "\\SI") {
        c = 0x0F;
    } else if (escape == "\\DLE") {
        c = 0x10;
    } else if (escape == "\\DC1") {
        c = 0x11;
    } else if (escape == "\\DC2") {
        c = 0x12;
    } else if (escape == "\\DC3") {
        c = 0x13;
    } else if (escape == "\\DC4") {
        c = 0x14;
    } else if (escape == "\\NAK") {
        c = 0x15;
    } else if (escape == "\\SYN") {
        c = 0x16;
    } else if (escape == "\\ETB") {
        c = 0x17;
    } else if (escape == "\\CAN") {
        c = 0x18;
    } else if (escape == "\\EM") {
        c = 0x19;
    } else if (escape == "\\SUB") {
        c = 0x1A;
    } else if (escape == "\\ESC") {
        c = 0x1B;
    } else if (escape == "\\FS") {
        c = 0x1C;
    } else if (escape == "\\GS") {
        c = 0x1D;
    } else if (escape == "\\RS") {
        c = 0x1E;
    } else if (escape == "\\US") {
        c = 0x1F;
    } else if (escape == "\\SP") {
        c = 0x20;
    } else if (escape == "\\DEL") {
        c = 0x7F;
    } else if (escape == "\\^@") {
        c = 0x00;
    } else if (escape == "\\^[") {
        c = 0x1B;
    } else if (escape == "\\^\\") {
        c = 0x1C;
    } else if (escape == "\\^]") {
        c = 0x1D;
    } else if (escape == "\\^^") {
        c = 0x1E;
    } else if (escape == "\\^_") {
        c = 0x1F;
    } else if (escape[1] == '^' && isupper(escape[2])) {
        c = escape[2] - 0x40;
    }

    if (c < 0 || c > 127) {
        throw yy::parser::syntax_error(loc, "Invalid escape sequence: " + escape);
    }
    return c;
}

yy::parser::symbol_type parse_character(const std::string &s, const yy::parser::location_type& loc) {
    if (s[1] == '\\') {
        return yy::parser::make_CHAR(parse_escape(s.substr(1, s.size() - 2), loc), loc);
    }
    return yy::parser::make_CHAR(s[1], loc);
}

void reset_start_condition() {
    BEGIN(INITIAL);
}
