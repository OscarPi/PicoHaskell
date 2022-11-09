%{ /* -*- C++ -*- */
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>
#include "parser/driver.hpp"
#include "parser/parser.hpp"
%}
%option noyywrap nounput noinput batch debug stack
%{
yy::parser::symbol_type make_NUMBER (const std::string &s, const yy::parser::location_type& loc);
%}

%x comment
%x string

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
WHITECHAR   [\t \v]

VARID       {SMALL}({SMALL}|{LARGE}|{DIGIT}|\')*
CONID       {LARGE}({SMALL}|{LARGE}|{DIGIT}|\')*
VARSYM      [!#\$%&\*\+\.\/\<=\>\?@\\\^\|\-~]{SYMBOL}*
CONSYM      ":"{SYMBOL}*

COMMENT     "--""-"*(({SMALL}|{LARGE}|{DIGIT}|{SPECIAL}|\"|\'|" "|\t){ANY}*)?{NEWLINE}
OPENCOM     "{""-"+
CLOSECOM    "-"+"}"
INCOM       {SMALL}|{LARGE}|{DIGIT}|[!#\$%&\*\+\.\/\<=\>\?@\\\^\|~\:\(\)\,\;\[\]\`\"\'\t \v]
%{
// Code run each time a pattern is matched.
#define YY_USER_ACTION  loc.columns(yyleng);
%}
%%
%{
// A handy shortcut to the location held by the Driver.
yy::location& loc = drv.location;
// Code run each time yylex is called.
loc.step();
%}
{WHITECHAR} loc.step();
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
{VARID}     return yy::parser::make_VARID(yytext, loc);
{CONID}     return yy::parser::make_CONID(yytext, loc);
{VARSYM}    return yy::parser::make_VARSYM(yytext, loc);
{CONSYM}    return yy::parser::make_CONSYM(yytext, loc);
{OPENCOM}   yy_push_state(comment); loc.step();
<comment>{
    {OPENCOM}               yy_push_state(comment); loc.step();
    {CLOSECOM}              yy_pop_state(); loc.step();
    {NEWLINE}               loc.lines(1); loc.step();
    ({INCOM}|"}")*          loc.step();
    "-"+{INCOM}*            loc.step();
    "{"+({INCOM}|"}")*      loc.step();
}
<<EOF>>     return yy::parser::make_YYEOF(loc);
%%
void Driver::scan_begin() {
  yy_flex_debug = trace_scanning;
  if (file.empty () || file == "-")
    yyin = stdin;
  else if (!(yyin = fopen (file.c_str (), "r")))
    {
      std::cerr << "cannot open " << file << ": " << strerror (errno) << '\n';
      exit (EXIT_FAILURE);
    }
}

void Driver::scan_end () {
  fclose (yyin);
}

void reset_start_condition() {
    BEGIN(INITIAL);
}
