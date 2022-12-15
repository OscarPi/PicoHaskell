#include <gtest/gtest.h>
#include <vector>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "parser/driver.hpp"

void reset_start_condition();

std::vector<yy::parser::symbol_type> lex_string(const char* str) {
    std::vector<yy::parser::symbol_type> result;
    Driver drv;
    YY_BUFFER_STATE buffer = yy_scan_string(str);
    yy_switch_to_buffer(buffer);
    reset_start_condition();
    while (true) {
        result.push_back(yylex(drv));
        if (result.back().kind() == yy::parser::symbol_kind_type::S_YYEOF) {
            result.pop_back();
            break;
        }
    }
    yy_delete_buffer(buffer);
    return result;
}

#define EXPECT_SYMBOL(str, sym) {     \
    auto result = lex_string(str);    \
    ASSERT_EQ(result.size(), 1);      \
    EXPECT_EQ(result[0].kind(), sym); \
}

#define EXPECT_INTEGER(str, i) {                                          \
    auto result = lex_string(str);                                        \
    ASSERT_EQ(result.size(), 1);                                          \
    ASSERT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_INTEGER); \
    EXPECT_EQ(result[0].value.as<int>(), i);                              \
}

#define EXPECT_FLOAT(str, f) {                                          \
    auto result = lex_string(str);                                      \
    ASSERT_EQ(result.size(), 1);                                        \
    ASSERT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_FLOAT); \
    EXPECT_DOUBLE_EQ(result[0].value.as<double>(), f);                  \
}

#define EXPECT_CHAR(str, c) {                                          \
    auto result = lex_string(str);                                     \
    ASSERT_EQ(result.size(), 1);                                       \
    ASSERT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_CHAR); \
    EXPECT_EQ(result[0].value.as<char>(), c);                          \
}

#define EXPECT_STRING(str, s) {                                          \
    auto result = lex_string(str);                                       \
    ASSERT_EQ(result.size(), 1);                                         \
    ASSERT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_STRING); \
    EXPECT_EQ(result[0].value.as<std::string>(), s);                     \
}

TEST(Lexer, RecognisesKeywords) {
    EXPECT_SYMBOL("case", yy::parser::symbol_kind_type::S_CASE);
    EXPECT_SYMBOL("class", yy::parser::symbol_kind_type::S_CLASS);
    EXPECT_SYMBOL("data", yy::parser::symbol_kind_type::S_DATA);
    EXPECT_SYMBOL("default", yy::parser::symbol_kind_type::S_DEFAULT);
    EXPECT_SYMBOL("deriving", yy::parser::symbol_kind_type::S_DERIVING);
    EXPECT_SYMBOL("do", yy::parser::symbol_kind_type::S_DO);
    EXPECT_SYMBOL("else", yy::parser::symbol_kind_type::S_ELSE);
    EXPECT_SYMBOL("foreign", yy::parser::symbol_kind_type::S_FOREIGN);
    EXPECT_SYMBOL("if", yy::parser::symbol_kind_type::S_IF);
    EXPECT_SYMBOL("import", yy::parser::symbol_kind_type::S_IMPORT);
    EXPECT_SYMBOL("in", yy::parser::symbol_kind_type::S_IN);
    EXPECT_SYMBOL("infix", yy::parser::symbol_kind_type::S_INFIX);
    EXPECT_SYMBOL("infixl", yy::parser::symbol_kind_type::S_INFIXL);
    EXPECT_SYMBOL("infixr", yy::parser::symbol_kind_type::S_INFIXR);
    EXPECT_SYMBOL("instance", yy::parser::symbol_kind_type::S_INSTANCE);
    EXPECT_SYMBOL("let", yy::parser::symbol_kind_type::S_LET);
    EXPECT_SYMBOL("module", yy::parser::symbol_kind_type::S_MODULE);
    EXPECT_SYMBOL("newtype", yy::parser::symbol_kind_type::S_NEWTYPE);
    EXPECT_SYMBOL("of", yy::parser::symbol_kind_type::S_OF);
    EXPECT_SYMBOL("then", yy::parser::symbol_kind_type::S_THEN);
    EXPECT_SYMBOL("type", yy::parser::symbol_kind_type::S_TYPE);
    EXPECT_SYMBOL("where", yy::parser::symbol_kind_type::S_WHERE);
}

TEST(Lexer, RecognisesOps) {
    EXPECT_SYMBOL("_", yy::parser::symbol_kind_type::S__);
    EXPECT_SYMBOL("..", yy::parser::symbol_kind_type::S_DOTDOT);
    EXPECT_SYMBOL(":", yy::parser::symbol_kind_type::S_COLON);
    EXPECT_SYMBOL("::", yy::parser::symbol_kind_type::S_HASTYPE);
    EXPECT_SYMBOL("=", yy::parser::symbol_kind_type::S_EQUALS);
    EXPECT_SYMBOL("\\", yy::parser::symbol_kind_type::S_BACKSLASH);
    EXPECT_SYMBOL("|", yy::parser::symbol_kind_type::S_PIPE);
    EXPECT_SYMBOL("<-", yy::parser::symbol_kind_type::S_LEFTARROW);
    EXPECT_SYMBOL("->", yy::parser::symbol_kind_type::S_RIGHTARROW);
    EXPECT_SYMBOL("@", yy::parser::symbol_kind_type::S_AT);
    EXPECT_SYMBOL("~", yy::parser::symbol_kind_type::S_TILDE);
    EXPECT_SYMBOL("=>", yy::parser::symbol_kind_type::S_IMPLIES);
    EXPECT_SYMBOL("(", yy::parser::symbol_kind_type::S_LEFTBRACKET);
    EXPECT_SYMBOL(")", yy::parser::symbol_kind_type::S_RIGHTBRACKET);
    EXPECT_SYMBOL(",", yy::parser::symbol_kind_type::S_COMMA);
    EXPECT_SYMBOL(";", yy::parser::symbol_kind_type::S_SEMICOLON);
    EXPECT_SYMBOL("[", yy::parser::symbol_kind_type::S_LEFTCROTCHET);
    EXPECT_SYMBOL("]", yy::parser::symbol_kind_type::S_RIGHTCROTCHET);
    EXPECT_SYMBOL("`", yy::parser::symbol_kind_type::S_BACKTICK);
    EXPECT_SYMBOL("{", yy::parser::symbol_kind_type::S_LEFTBRACE);
    EXPECT_SYMBOL("}", yy::parser::symbol_kind_type::S_RIGHTBRACE);
    EXPECT_SYMBOL("+", yy::parser::symbol_kind_type::S_PLUS);
    EXPECT_SYMBOL("-", yy::parser::symbol_kind_type::S_MINUS);
    EXPECT_SYMBOL("*", yy::parser::symbol_kind_type::S_TIMES);
    EXPECT_SYMBOL("/", yy::parser::symbol_kind_type::S_DIVIDE);
    EXPECT_SYMBOL("==", yy::parser::symbol_kind_type::S_EQUALITY);
    EXPECT_SYMBOL("/=", yy::parser::symbol_kind_type::S_INEQUALITY);
    EXPECT_SYMBOL("<", yy::parser::symbol_kind_type::S_LT);
    EXPECT_SYMBOL("<=", yy::parser::symbol_kind_type::S_LTE);
    EXPECT_SYMBOL(">", yy::parser::symbol_kind_type::S_GT);
    EXPECT_SYMBOL(">=", yy::parser::symbol_kind_type::S_GTE);
    EXPECT_SYMBOL("&&", yy::parser::symbol_kind_type::S_AND);
    EXPECT_SYMBOL("||", yy::parser::symbol_kind_type::S_OR);
}

TEST(Lexer, RecognisesVARID) {
    EXPECT_SYMBOL("iff", yy::parser::symbol_kind_type::S_VARID);
    EXPECT_SYMBOL("ndefault", yy::parser::symbol_kind_type::S_VARID);
    EXPECT_SYMBOL("lotsofcases", yy::parser::symbol_kind_type::S_VARID);
    EXPECT_SYMBOL("zsS''123'", yy::parser::symbol_kind_type::S_VARID);
    EXPECT_SYMBOL("a", yy::parser::symbol_kind_type::S_VARID);
    EXPECT_THROW(lex_string("'a"), yy::parser::syntax_error);
}

TEST(Lexer, RecognisesCONID) {
    EXPECT_SYMBOL("Iff", yy::parser::symbol_kind_type::S_CONID);
    EXPECT_SYMBOL("Ndefault", yy::parser::symbol_kind_type::S_CONID);
    EXPECT_SYMBOL("Lotsofcases", yy::parser::symbol_kind_type::S_CONID);
    EXPECT_SYMBOL("ZsS''123'", yy::parser::symbol_kind_type::S_CONID);
    EXPECT_SYMBOL("A", yy::parser::symbol_kind_type::S_CONID);
}

TEST(Lexer, HandlesWhitespace) {
    auto result = lex_string("if     I\n\n\v\v\v\t\tam + +");
    ASSERT_EQ(result.size(), 7);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_IF);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_CONID);
    EXPECT_EQ(result[2].kind(), yy::parser::symbol_kind_type::S_NEWLINE);
    EXPECT_EQ(result[3].kind(), yy::parser::symbol_kind_type::S_NEWLINE);
    EXPECT_EQ(result[4].kind(), yy::parser::symbol_kind_type::S_VARID);
    EXPECT_EQ(result[5].kind(), yy::parser::symbol_kind_type::S_PLUS);
    EXPECT_EQ(result[6].kind(), yy::parser::symbol_kind_type::S_PLUS);
}

TEST(Lexer, IgnoresSingleLineComments) {
    auto result = lex_string("--\n");
    EXPECT_EQ(result.size(), 0);

    EXPECT_SYMBOL("--a#!2334\nif", yy::parser::symbol_kind_type::S_IF);

    result = lex_string("default--a#!2334\nif");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_DEFAULT);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_IF);

    result = lex_string("default -- a#!\\!@ if 2334\nif");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_DEFAULT);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_IF);
}

TEST(Lexer, IgnoresMultiLineComments) {
    EXPECT_SYMBOL("{-{}--{}{}---{}{}-{}}}}{{{}-}if", yy::parser::symbol_kind_type::S_IF);

    auto result = lex_string("{-if\nif-}");
    EXPECT_EQ(result.size(), 0);

    EXPECT_SYMBOL("{- if {- if {- if -} -} if \n\n\n-}default",  yy::parser::symbol_kind_type::S_DEFAULT);
}

TEST(Lexer, HandlesIntegerLiterals) {
    EXPECT_INTEGER("0123456", 123456);
    EXPECT_INTEGER("0x123456", 0x123456);
    EXPECT_INTEGER("0XABCDEF", 0xABCDEF);
    EXPECT_INTEGER("0o1234567", 01234567);
    EXPECT_INTEGER("0O12", 012);
}

TEST(Lexer, HandlesFloatLiterals) {
    EXPECT_FLOAT("3.14", 3.14);
    EXPECT_FLOAT("3.14e10", 3.14e10);
    EXPECT_FLOAT("3.14e+10", 3.14e+10);
    EXPECT_FLOAT("3.14e-10", 3.14e-10);
    EXPECT_FLOAT("3.14E-10", 3.14E-10);
    EXPECT_FLOAT("3.14E+10", 3.14E+10);
    EXPECT_FLOAT("3.14E10", 3.14E10);
    EXPECT_FLOAT("2e10", 2e10);
    EXPECT_FLOAT("2e+10", 2e+10);
    EXPECT_FLOAT("2e-10", 2e-10);
    EXPECT_FLOAT("2E10", 2E10);
    EXPECT_FLOAT("2E+10", 2E+10);
    EXPECT_FLOAT("2E-10", 2E-10);
}

TEST(Lexer, HandlesCharLiterals) {
    EXPECT_CHAR("'a'", 'a');
    EXPECT_CHAR("'\\0'", 0x00);
    EXPECT_CHAR("'\\a'", 0x07);
    EXPECT_CHAR("'\\b'", 0x08);
    EXPECT_CHAR("'\\f'", 0x0C);
    EXPECT_CHAR("'\\n'", 0x0A);
    EXPECT_CHAR("'\\r'", 0x0D);
    EXPECT_CHAR("'\\t'", 0x09);
    EXPECT_CHAR("'\\v'", 0x0B);
    EXPECT_CHAR("'\\\"'", 0x22);
    EXPECT_CHAR("'\\''", 0x27);
    EXPECT_CHAR("'\\\\'", 0x5C);
    EXPECT_CHAR("'\\NUL'", 0x00);
    EXPECT_CHAR("'\\SOH'", 0x01);
    EXPECT_CHAR("'\\STX'", 0x02);
    EXPECT_CHAR("'\\ETX'", 0x03);
    EXPECT_CHAR("'\\EOT'", 0x04);
    EXPECT_CHAR("'\\ENQ'", 0x05);
    EXPECT_CHAR("'\\ACK'", 0x06);
    EXPECT_CHAR("'\\BEL'", 0x07);
    EXPECT_CHAR("'\\BS'", 0x08);
    EXPECT_CHAR("'\\HT'", 0x09);
    EXPECT_CHAR("'\\LF'", 0x0A);
    EXPECT_CHAR("'\\VT'", 0x0B);
    EXPECT_CHAR("'\\FF'", 0x0C);
    EXPECT_CHAR("'\\CR'", 0x0D);
    EXPECT_CHAR("'\\SO'", 0x0E);
    EXPECT_CHAR("'\\SI'", 0x0F);
    EXPECT_CHAR("'\\DLE'", 0x10);
    EXPECT_CHAR("'\\DC1'", 0x11);
    EXPECT_CHAR("'\\DC2'", 0x12);
    EXPECT_CHAR("'\\DC3'", 0x13);
    EXPECT_CHAR("'\\DC4'", 0x14);
    EXPECT_CHAR("'\\NAK'", 0x15);
    EXPECT_CHAR("'\\SYN'", 0x16);
    EXPECT_CHAR("'\\ETB'", 0x17);
    EXPECT_CHAR("'\\CAN'", 0x18);
    EXPECT_CHAR("'\\EM'", 0x19);
    EXPECT_CHAR("'\\SUB'", 0x1A);
    EXPECT_CHAR("'\\ESC'", 0x1B);
    EXPECT_CHAR("'\\FS'", 0x1C);
    EXPECT_CHAR("'\\GS'", 0x1D);
    EXPECT_CHAR("'\\RS'", 0x1E);
    EXPECT_CHAR("'\\US'", 0x1F);
    EXPECT_CHAR("'\\SP'", 0x20);
    EXPECT_CHAR("'\\DEL'", 0x7F);
    EXPECT_CHAR("'\\^@'", 0x00);
    EXPECT_CHAR("'\\^A'", 0x01);
    EXPECT_CHAR("'\\^C'", 0x03);
    EXPECT_CHAR("'\\^Z'", 0x1A);
    EXPECT_CHAR("'\\^['", 0x1B);
    EXPECT_CHAR("'\\^\\'", 0x1C);
    EXPECT_CHAR("'\\^]'", 0x1D);
    EXPECT_CHAR("'\\^^'", 0x1E);
    EXPECT_CHAR("'\\^_'", 0x1F);
    EXPECT_CHAR("'\\123'", 123);
    EXPECT_CHAR("'\\x12'", 0x12);
    EXPECT_CHAR("'\\o12'", 012);

    EXPECT_THROW(lex_string("'\\O12'"), yy::parser::syntax_error);
    EXPECT_THROW(lex_string("'\\X12'"), yy::parser::syntax_error);
    EXPECT_THROW(lex_string("'\\1234'"), yy::parser::syntax_error);
    EXPECT_THROW(lex_string("'\\&'"), yy::parser::syntax_error);
    EXPECT_THROW(lex_string("''"), yy::parser::syntax_error);
}

TEST(Lexer, HandlesStringLiterals) {
    auto result = lex_string(R"("abc123" "abc143")");
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_STRING);
    ASSERT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_STRING);
    EXPECT_EQ(result[0].value.as<std::string>(), "abc123");
    EXPECT_EQ(result[1].value.as<std::string>(), "abc143");

    EXPECT_STRING(R"("a bc\&123")", "a bc123");
    EXPECT_STRING(R"("")", "");
    EXPECT_STRING(R"("\&")", "")
    EXPECT_STRING(R"("\1\2\3")", "\1\2\3");
    EXPECT_STRING("\"hi\\   \t\v\n\r\n\\bye\"", "hibye");

    EXPECT_THROW(lex_string(R"("\poopoo\")"), yy::parser::syntax_error);
}

TEST(Lexer, TracksLocations) {
    auto result = lex_string("    if default");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_IF);
    EXPECT_EQ(result[0].location.begin.line, 1);
    EXPECT_EQ(result[0].location.begin.column, 5);
    EXPECT_EQ(result[0].location.end.line, 1);
    EXPECT_EQ(result[0].location.end.column, 7);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_DEFAULT);
    EXPECT_EQ(result[1].location.begin.line, 1);
    EXPECT_EQ(result[1].location.begin.column, 8);
    EXPECT_EQ(result[1].location.end.line, 1);
    EXPECT_EQ(result[1].location.end.column, 15);

    result = lex_string("\tif\n  \tif\tdefault\"hey\\\n\\howdy\"\ndefault{-\n\n\n-}\n--sup\n\tif");
    ASSERT_EQ(result.size(), 9);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_IF);
    EXPECT_EQ(result[0].location.begin.line, 1);
    EXPECT_EQ(result[0].location.begin.column, 9);
    EXPECT_EQ(result[0].location.end.line, 1);
    EXPECT_EQ(result[0].location.end.column, 11);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_NEWLINE);
    EXPECT_EQ(result[1].location.begin.line, 1);
    EXPECT_EQ(result[1].location.begin.column, 11);
    EXPECT_EQ(result[1].location.end.line, 2);
    EXPECT_EQ(result[1].location.end.column, 1);
    EXPECT_EQ(result[2].kind(), yy::parser::symbol_kind_type::S_IF);
    EXPECT_EQ(result[2].location.begin.line, 2);
    EXPECT_EQ(result[2].location.begin.column, 9);
    EXPECT_EQ(result[2].location.end.line, 2);
    EXPECT_EQ(result[2].location.end.column, 11);
    EXPECT_EQ(result[3].kind(), yy::parser::symbol_kind_type::S_DEFAULT);
    EXPECT_EQ(result[3].location.begin.line, 2);
    EXPECT_EQ(result[3].location.begin.column, 17);
    EXPECT_EQ(result[3].location.end.line, 2);
    EXPECT_EQ(result[3].location.end.column, 24);
    EXPECT_EQ(result[4].kind(), yy::parser::symbol_kind_type::S_STRING);
    EXPECT_EQ(result[4].location.begin.line, 2);
    EXPECT_EQ(result[4].location.begin.column, 24);
    EXPECT_EQ(result[4].location.end.line, 3);
    EXPECT_EQ(result[4].location.end.column, 8);
    EXPECT_EQ(result[5].kind(), yy::parser::symbol_kind_type::S_NEWLINE);
    EXPECT_EQ(result[5].location.begin.line, 3);
    EXPECT_EQ(result[5].location.begin.column, 8);
    EXPECT_EQ(result[5].location.end.line, 4);
    EXPECT_EQ(result[5].location.end.column, 1);
    EXPECT_EQ(result[6].kind(), yy::parser::symbol_kind_type::S_DEFAULT);
    EXPECT_EQ(result[6].location.begin.line, 4);
    EXPECT_EQ(result[6].location.begin.column, 1);
    EXPECT_EQ(result[6].location.end.line, 4);
    EXPECT_EQ(result[6].location.end.column, 8);
    EXPECT_EQ(result[7].kind(), yy::parser::symbol_kind_type::S_NEWLINE);
    EXPECT_EQ(result[7].location.begin.line, 7);
    EXPECT_EQ(result[7].location.begin.column, 3);
    EXPECT_EQ(result[7].location.end.line, 8);
    EXPECT_EQ(result[7].location.end.column, 1);
    EXPECT_EQ(result[8].kind(), yy::parser::symbol_kind_type::S_IF);
    EXPECT_EQ(result[8].location.begin.line, 9);
    EXPECT_EQ(result[8].location.begin.column, 9);
    EXPECT_EQ(result[8].location.end.line, 9);
    EXPECT_EQ(result[8].location.end.column, 11);
}
