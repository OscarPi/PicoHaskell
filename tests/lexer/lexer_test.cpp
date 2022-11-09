#include <gtest/gtest.h>
#include <vector>
#include <variant>
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

#define EXPECT_SYMBOL(str, sym) {  \
    auto result = lex_string(str); \
    ASSERT_EQ(result.size(), 1);   \
    EXPECT_EQ(result[0].kind(), sym);     \
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

TEST(Lexer, RecognisesReserveOp) {
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
}

TEST(Lexer, RecognisesVARID) {
    EXPECT_SYMBOL("iff", yy::parser::symbol_kind_type::S_VARID);
    EXPECT_SYMBOL("ndefault", yy::parser::symbol_kind_type::S_VARID);
    EXPECT_SYMBOL("lotsofcases", yy::parser::symbol_kind_type::S_VARID);
    EXPECT_SYMBOL("zsS''123'", yy::parser::symbol_kind_type::S_VARID);
    EXPECT_SYMBOL("a", yy::parser::symbol_kind_type::S_VARID);
    //TODO: starting with '
}

TEST(Lexer, RecognisesCONID) {
    EXPECT_SYMBOL("Iff", yy::parser::symbol_kind_type::S_CONID);
    EXPECT_SYMBOL("Ndefault", yy::parser::symbol_kind_type::S_CONID);
    EXPECT_SYMBOL("Lotsofcases", yy::parser::symbol_kind_type::S_CONID);
    EXPECT_SYMBOL("ZsS''123'", yy::parser::symbol_kind_type::S_CONID);
    EXPECT_SYMBOL("A", yy::parser::symbol_kind_type::S_CONID);
    //TODO: starting with '
}

TEST(Lexer, RecognisesVARSYM) {
    EXPECT_SYMBOL("!", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("#", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("$", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("%", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("&", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("*", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("+", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL(".", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("/", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("<", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("=!", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL(">", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("?", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("@#:!", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("\\\\\\\\", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("^", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("||", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("-", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("~~#", yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_SYMBOL("--#\n", yy::parser::symbol_kind_type::S_VARSYM);
}

TEST(Lexer, RecognisesCONSYM) {
    EXPECT_SYMBOL(":::", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":!", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":#", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":$", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":%", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":&", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":*", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":+", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":.", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":/", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":<", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":=!", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":>", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":?", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":@#:!", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":\\\\\\\\", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":^", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":||", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":-", yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_SYMBOL(":~~#", yy::parser::symbol_kind_type::S_CONSYM);
}

TEST(Lexer, HandleIDMixedWithSYM) {
    auto result = lex_string("hello##Goodbye");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_VARID);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_EQ(result[2].kind(), yy::parser::symbol_kind_type::S_CONID);

    result = lex_string("##Goodbye");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_CONID);

    result = lex_string("hello##");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_VARID);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_VARSYM);

    result = lex_string("Hello##goodbye");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_CONID);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_EQ(result[2].kind(), yy::parser::symbol_kind_type::S_VARID);

    result = lex_string("##goodbye");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_VARSYM);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_VARID);

    result = lex_string("Hello##");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_CONID);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_VARSYM);
}

TEST(Lexer, IgnoresWhitespace) {
    auto result = lex_string("if     I\n\n\n\n\r\n\r\f\v\v\v\t\tam :$ $$");
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_IF);
    EXPECT_EQ(result[1].kind(), yy::parser::symbol_kind_type::S_CONID);
    EXPECT_EQ(result[2].kind(), yy::parser::symbol_kind_type::S_VARID);
    EXPECT_EQ(result[3].kind(), yy::parser::symbol_kind_type::S_CONSYM);
    EXPECT_EQ(result[4].kind(), yy::parser::symbol_kind_type::S_VARSYM);
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
    auto result = lex_string("0123456");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_INTEGER);
    EXPECT_EQ(result[0].value.as<int>(), 123456);

    result = lex_string("0x123456");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_INTEGER);
    EXPECT_EQ(result[0].value.as<int>(), 0x123456);

    result = lex_string("0XABCDEF");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_INTEGER);
    EXPECT_EQ(result[0].value.as<int>(), 0xABCDEF);

    result = lex_string("0o1234567");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_INTEGER);
    EXPECT_EQ(result[0].value.as<int>(), 01234567);

    result = lex_string("0O12");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind(), yy::parser::symbol_kind_type::S_INTEGER);
    EXPECT_EQ(result[0].value.as<int>(), 012);
}
