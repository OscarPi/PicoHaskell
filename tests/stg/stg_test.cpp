#include <gtest/gtest.h>
#include "test/test_utilities.hpp"
#include "stg/stg.hpp"

#define EXPECT_VARIABLE(lambda_form, v) {                                            \
    EXPECT_EQ((lambda_form)->free_variables.size(), 1);                              \
    EXPECT_EQ((lambda_form)->free_variables.count((v)), 1);                          \
    EXPECT_EQ((lambda_form)->argument_variables.size(), 0);                          \
    EXPECT_EQ((lambda_form)->updatable, true);                                       \
    ASSERT_EQ((lambda_form)->expr->get_form(), stgform::variable);                   \
    EXPECT_EQ(dynamic_cast<STGVariable*>((lambda_form)->expr.get())->name, (v));     \
}

TEST(STGTranslation, TranslatesVariables) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string_no_prelude("b = a", program.get());
    ASSERT_EQ(result, 0);
    auto translated = translate(program);
    EXPECT_VARIABLE(translated->bindings.at("b"), "a");
}

#define EXPECT_CHAR(lambda_form, c) {                                                \
    EXPECT_EQ((lambda_form)->free_variables.size(), 0);                              \
    EXPECT_EQ((lambda_form)->argument_variables.size(), 0);                          \
    EXPECT_EQ((lambda_form)->updatable, false);                                      \
    ASSERT_EQ((lambda_form)->expr->get_form(), stgform::literal);                    \
    EXPECT_EQ(                                                                       \
        std::get<char>(dynamic_cast<STGLiteral*>((lambda_form)->expr.get())->value), \
        (c));                                                                        \
}

TEST(STGTranslation, TranslatesLiterals) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string_no_prelude("b = 1", program.get());
    ASSERT_EQ(result, 0);
    auto translated = translate(program);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->updatable, false);
    ASSERT_EQ(translated->bindings.at("b")->expr->get_form(), stgform::literal);
    EXPECT_EQ(
            std::get<int>(dynamic_cast<STGLiteral*>(translated->bindings.at("b")->expr.get())->value),
            1);

    program = std::make_unique<Program>();
    result = parse_string_no_prelude("b = 'a'", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_CHAR(translated->bindings.at("b"), 'a');
}

TEST(STGTranslation, TranslatesAbstractions) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string_no_prelude("h a = \\a -> a", program.get());
    ASSERT_EQ(result, 0);
    auto translated = translate(program);
    EXPECT_EQ(translated->bindings.at("h")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("h")->argument_variables.size(), 2);
    EXPECT_EQ(translated->bindings.at("h")->argument_variables[0], ".0");
    EXPECT_EQ(translated->bindings.at("h")->argument_variables[1], ".1");
    EXPECT_EQ(translated->bindings.at("h")->updatable, false);
    ASSERT_EQ(translated->bindings.at("h")->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>((translated->bindings.at("h"))->expr.get())->name, ".1");
}

TEST(STGTranslation, TranslatesLet) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string_no_prelude(
            "x t = let { a=t; b=a; c=let { t='a' } in t; f=let { a=b; b=a } in 'b' } in let { g=f } in f",
            program.get());
    ASSERT_EQ(result, 0);
    auto translated = translate(program);
    EXPECT_EQ(translated->bindings.at("x")->free_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("x")->free_variables.count(".4"), 1);
    EXPECT_EQ(translated->bindings.at("x")->argument_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("x")->argument_variables[0], ".0");
    EXPECT_EQ(translated->bindings.at("x")->updatable, false);
    ASSERT_EQ(translated->bindings.at("x")->expr->get_form(), stgform::let);
    auto let = dynamic_cast<STGLet*>(translated->bindings.at("x")->expr.get());
    EXPECT_EQ(let->recursive, false);
    EXPECT_EQ(let->bindings.size(), 1);
    EXPECT_VARIABLE(let->bindings.at(".1"), ".0");
    ASSERT_EQ(let->expr->get_form(), stgform::let);
    let = dynamic_cast<STGLet*>(let->expr.get());
    EXPECT_EQ(let->recursive, false);
    EXPECT_EQ(let->bindings.size(), 1);
    EXPECT_VARIABLE(let->bindings.at(".2"), ".1");
    ASSERT_EQ(let->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(let->expr.get())->name, ".4");

    EXPECT_VARIABLE(translated->bindings.at(".3"), ".7");
    EXPECT_CHAR(translated->bindings.at(".4"), 'b');
    EXPECT_VARIABLE(translated->bindings.at(".5"), ".6");
    EXPECT_VARIABLE(translated->bindings.at(".6"), ".5");
    EXPECT_CHAR(translated->bindings.at(".7"), 'a');
    EXPECT_VARIABLE(translated->bindings.at(".8"), ".4");
}

TEST(STGTranslation, TranslatesConstructors) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string_no_prelude("data T = Test\n;b = Test", program.get());
    ASSERT_EQ(result, 0);
    auto translated = translate(program);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->updatable, false);
    ASSERT_EQ(translated->bindings.at("b")->expr->get_form(), stgform::constructor);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at("b")->expr.get())->arguments.size(), 0);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at("b")->expr.get())->constructor_name, "Test");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude("data T = Test T\n;b = Test", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->argument_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("b")->argument_variables[0], ".0");
    EXPECT_EQ(translated->bindings.at("b")->updatable, false);
    ASSERT_EQ(translated->bindings.at("b")->expr->get_form(), stgform::let);
    auto let = dynamic_cast<STGLet*>(translated->bindings.at("b")->expr.get());
    EXPECT_EQ(let->recursive, false);
    EXPECT_EQ(let->bindings.size(), 1);
    EXPECT_EQ(let->bindings.at(".1")->free_variables.size(), 1);
    EXPECT_EQ(let->bindings.at(".1")->free_variables.count(".0"), 1);
    EXPECT_EQ(let->bindings.at(".1")->argument_variables.size(), 0);
    EXPECT_EQ(let->bindings.at(".1")->updatable, false);
    ASSERT_EQ(let->bindings.at(".1")->expr->get_form(), stgform::constructor);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(let->bindings.at(".1")->expr.get())->arguments.size(), 1);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(let->bindings.at(".1")->expr.get())->arguments[0], ".0");
    EXPECT_EQ(dynamic_cast<STGConstructor*>(let->bindings.at(".1")->expr.get())->constructor_name, "Test");
    ASSERT_EQ(let->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(let->expr.get())->name, ".1");
}

TEST(STGTranslation, TranslatesApplications) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string_no_prelude("data T = Test T T | Nil\n;b = Test Nil", program.get());
    ASSERT_EQ(result, 0);
    auto translated = translate(program);
    EXPECT_EQ(translated->bindings.at(".0")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at(".0")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at(".0")->updatable, false);
    ASSERT_EQ(translated->bindings.at(".0")->expr->get_form(), stgform::constructor);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at(".0")->expr.get())->constructor_name, "Nil");
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at(".0")->expr.get())->arguments.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count(".0"), 1);
    EXPECT_EQ(translated->bindings.at("b")->argument_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("b")->argument_variables[0], ".1");
    EXPECT_EQ(translated->bindings.at("b")->updatable, false);
    ASSERT_EQ(translated->bindings.at("b")->expr->get_form(), stgform::let);
    auto let = dynamic_cast<STGLet*>(translated->bindings.at("b")->expr.get());
    EXPECT_EQ(let->recursive, false);
    EXPECT_EQ(let->bindings.size(), 1);
    EXPECT_EQ(let->bindings.at(".2")->free_variables.size(), 2);
    EXPECT_EQ(let->bindings.at(".2")->free_variables.count(".0"), 1);
    EXPECT_EQ(let->bindings.at(".2")->free_variables.count(".1"), 1);
    EXPECT_EQ(let->bindings.at(".2")->argument_variables.size(), 0);
    EXPECT_EQ(let->bindings.at(".2")->updatable, false);
    ASSERT_EQ(let->bindings.at(".2")->expr->get_form(), stgform::constructor);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(let->bindings.at(".2")->expr.get())->arguments.size(), 2);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(let->bindings.at(".2")->expr.get())->arguments[0], ".0");
    EXPECT_EQ(dynamic_cast<STGConstructor*>(let->bindings.at(".2")->expr.get())->arguments[1], ".1");
    EXPECT_EQ(dynamic_cast<STGConstructor*>(let->bindings.at(".2")->expr.get())->constructor_name, "Test");
    ASSERT_EQ(let->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(let->expr.get())->name, ".2");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude("data T = Test T | Nil | Nill\n;b = Test Nil Nill", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.at(".0")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at(".0")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at(".0")->updatable, false);
    ASSERT_EQ(translated->bindings.at(".0")->expr->get_form(), stgform::constructor);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at(".0")->expr.get())->constructor_name, "Nill");
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at(".0")->expr.get())->arguments.size(), 0);
    EXPECT_EQ(translated->bindings.at(".1")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at(".1")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at(".1")->updatable, false);
    ASSERT_EQ(translated->bindings.at(".1")->expr->get_form(), stgform::constructor);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at(".1")->expr.get())->constructor_name, "Nil");
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at(".1")->expr.get())->arguments.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.size(), 2);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count(".0"), 1);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count(".1"), 1);
    EXPECT_EQ(translated->bindings.at("b")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->updatable, false);
    ASSERT_EQ(translated->bindings.at("b")->expr->get_form(), stgform::constructor);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at("b")->expr.get())->constructor_name, "Test");
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at("b")->expr.get())->arguments.size(), 2);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at("b")->expr.get())->arguments[0], ".1");
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated->bindings.at("b")->expr.get())->arguments[1], ".0");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude("b = a b c d", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.size(), 4);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count("a"), 1);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count("b"), 1);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count("c"), 1);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count("d"), 1);
    EXPECT_EQ(translated->bindings.at("b")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->updatable, true);
    ASSERT_EQ(translated->bindings.at("b")->expr->get_form(), stgform::application);
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at("b")->expr.get())->lhs, "a");
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at("b")->expr.get())->arguments.size(), 3);
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at("b")->expr.get())->arguments[0], "b");
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at("b")->expr.get())->arguments[1], "c");
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at("b")->expr.get())->arguments[2], "d");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude(
            "b = (let { t = f } in (f g)) (let { q = r } in (q t)) c d",
            program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_VARIABLE(translated->bindings.at(".0"), "r");
    EXPECT_EQ(translated->bindings.at(".1")->free_variables.size(), 2);
    EXPECT_EQ(translated->bindings.at(".1")->free_variables.count(".0"), 1);
    EXPECT_EQ(translated->bindings.at(".1")->free_variables.count("t"), 1);
    EXPECT_EQ(translated->bindings.at(".1")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at(".1")->updatable, true);
    ASSERT_EQ(translated->bindings.at(".1")->expr->get_form(), stgform::application);
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at(".1")->expr.get())->lhs, ".0");
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at(".1")->expr.get())->arguments.size(), 1);
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at(".1")->expr.get())->arguments[0], "t");
    EXPECT_VARIABLE(translated->bindings.at(".2"), "f");
    EXPECT_EQ(translated->bindings.at(".3")->free_variables.size(), 2);
    EXPECT_EQ(translated->bindings.at(".3")->free_variables.count("f"), 1);
    EXPECT_EQ(translated->bindings.at(".3")->free_variables.count("g"), 1);
    EXPECT_EQ(translated->bindings.at(".3")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at(".3")->updatable, true);
    ASSERT_EQ(translated->bindings.at(".3")->expr->get_form(), stgform::application);
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at(".3")->expr.get())->lhs, "f");
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at(".3")->expr.get())->arguments.size(), 1);
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at(".3")->expr.get())->arguments[0], "g");
    EXPECT_EQ(translated->bindings.at("b")->free_variables.size(), 4);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count(".3"), 1);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count(".1"), 1);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count("c"), 1);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count("d"), 1);
    EXPECT_EQ(translated->bindings.at("b")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->updatable, true);
    ASSERT_EQ(translated->bindings.at("b")->expr->get_form(), stgform::application);
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at("b")->expr.get())->lhs, ".3");
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at("b")->expr.get())->arguments.size(), 3);
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at("b")->expr.get())->arguments[0], ".1");
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at("b")->expr.get())->arguments[1], "c");
    EXPECT_EQ(dynamic_cast<STGApplication*>(translated->bindings.at("b")->expr.get())->arguments[2], "d");
}

TEST(STGTranslation, TranslatesCase) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string_no_prelude("a = case let { x = 1 } in x of { _ -> 'a' }", program.get());
    ASSERT_EQ(result, 0);
    auto translated = translate(program);
    EXPECT_EQ(translated->bindings.size(), 1);
    EXPECT_CHAR(translated->bindings.at("a"), 'a');

    program = std::make_unique<Program>();
    result = parse_string_no_prelude("a = case let { x = 'b' } in x of { x@_ -> 'a' }", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.size(), 3);
    EXPECT_CHAR(translated->bindings.at("a"), 'a');
    EXPECT_CHAR(translated->bindings.at(".0"), 'b');
    EXPECT_VARIABLE(translated->bindings.at(".1"), ".0");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude("a = case 'a' of { a -> a }", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.size(), 2);
    EXPECT_CHAR(translated->bindings.at(".0"), 'a');
    EXPECT_VARIABLE(translated->bindings.at("a"), ".0");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude("a = case 'a' of { x@a -> x ; _ -> 'b' }", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.size(), 2);
    EXPECT_CHAR(translated->bindings.at(".0"), 'a');
    EXPECT_VARIABLE(translated->bindings.at("a"), ".0");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude(
            "a = case 'a' of { 'a' -> '0' ; 'b' -> let { a = 'g' } in '1' }",
            program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_CHAR(translated->bindings.at(".0"), 'g');
    EXPECT_EQ(translated->bindings.at("a")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("a")->updatable, true);
    ASSERT_EQ(translated->bindings.at("a")->expr->get_form(), stgform::literalcase);
    auto cAsE = dynamic_cast<STGLiteralCase*>(translated->bindings.at("a")->expr.get());
    ASSERT_EQ(cAsE->expr->get_form(), stgform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<STGLiteral*>(cAsE->expr.get())->value), 'a');
    ASSERT_EQ(cAsE->alts.size(), 2);
    EXPECT_EQ(std::get<char>(cAsE->alts[0].first.value), 'a');
    ASSERT_EQ(cAsE->alts[0].second->get_form(), stgform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<STGLiteral*>(cAsE->alts[0].second.get())->value), '0');
    EXPECT_EQ(std::get<char>(cAsE->alts[1].first.value), 'b');
    ASSERT_EQ(cAsE->alts[1].second->get_form(), stgform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<STGLiteral*>(cAsE->alts[1].second.get())->value), '1');
    EXPECT_EQ(cAsE->default_var, "");
    ASSERT_EQ(cAsE->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->default_expr.get())->name, "case_error");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude("a = case 'a' of { k@'a' -> k ; 'b' -> '1' }", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_CHAR(translated->bindings.at(".0"), 'a');
    EXPECT_EQ(translated->bindings.at("a")->free_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("a")->free_variables.count(".0"), 1);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("a")->updatable, true);
    ASSERT_EQ(translated->bindings.at("a")->expr->get_form(), stgform::literalcase);
    cAsE = dynamic_cast<STGLiteralCase*>(translated->bindings.at("a")->expr.get());
    ASSERT_EQ(cAsE->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->expr.get())->name, ".0");
    ASSERT_EQ(cAsE->alts.size(), 2);
    EXPECT_EQ(std::get<char>(cAsE->alts[0].first.value), 'a');
    ASSERT_EQ(cAsE->alts[0].second->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->alts[0].second.get())->name, ".0");
    EXPECT_EQ(std::get<char>(cAsE->alts[1].first.value), 'b');
    ASSERT_EQ(cAsE->alts[1].second->get_form(), stgform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<STGLiteral*>(cAsE->alts[1].second.get())->value), '1');
    EXPECT_EQ(cAsE->default_var, "");
    ASSERT_EQ(cAsE->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->default_expr.get())->name, "case_error");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude("a = case 'a' of { 'a' -> k ; _ -> '1' }", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.at("a")->free_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("a")->free_variables.count("k"), 1);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("a")->updatable, true);
    ASSERT_EQ(translated->bindings.at("a")->expr->get_form(), stgform::literalcase);
    cAsE = dynamic_cast<STGLiteralCase*>(translated->bindings.at("a")->expr.get());
    ASSERT_EQ(cAsE->expr->get_form(), stgform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<STGLiteral*>(cAsE->expr.get())->value), 'a');
    ASSERT_EQ(cAsE->alts.size(), 1);
    EXPECT_EQ(std::get<char>(cAsE->alts[0].first.value), 'a');
    ASSERT_EQ(cAsE->alts[0].second->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->alts[0].second.get())->name, "k");
    EXPECT_EQ(cAsE->default_var, "");
    ASSERT_EQ(cAsE->default_expr->get_form(), stgform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<STGLiteral*>(cAsE->default_expr.get())->value), '1');


    program = std::make_unique<Program>();
    result = parse_string_no_prelude("a = case 'a' of { 'a' -> k ; k -> k }", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.at("a")->free_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("a")->free_variables.count("k"), 1);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("a")->updatable, true);
    ASSERT_EQ(translated->bindings.at("a")->expr->get_form(), stgform::literalcase);
    cAsE = dynamic_cast<STGLiteralCase*>(translated->bindings.at("a")->expr.get());
    ASSERT_EQ(cAsE->expr->get_form(), stgform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<STGLiteral*>(cAsE->expr.get())->value), 'a');
    ASSERT_EQ(cAsE->alts.size(), 1);
    EXPECT_EQ(std::get<char>(cAsE->alts[0].first.value), 'a');
    ASSERT_EQ(cAsE->alts[0].second->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->alts[0].second.get())->name, "k");
    EXPECT_EQ(cAsE->default_var, "k");
    ASSERT_EQ(cAsE->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->default_expr.get())->name, "k");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude(
            "data Hi = Nil | Nill\n;a = case 'a' of { Nil -> 'a' ; Nill -> 'b' ; Nil -> 'c' }",
            program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.at("a")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("a")->updatable, true);
    ASSERT_EQ(translated->bindings.at("a")->expr->get_form(), stgform::algebraiccase);
    auto CaSe = dynamic_cast<STGAlgebraicCase*>(translated->bindings.at("a")->expr.get());
    ASSERT_EQ(CaSe->expr->get_form(), stgform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<STGLiteral*>(CaSe->expr.get())->value), 'a');
    ASSERT_EQ(CaSe->alts.size(), 2);
    EXPECT_EQ(CaSe->alts[0].first.constructor_name, "Nil");
    EXPECT_EQ(CaSe->alts[0].first.variables.size(), 0);
    ASSERT_EQ(CaSe->alts[0].second->get_form(), stgform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<STGLiteral*>(CaSe->alts[0].second.get())->value), 'a');
    EXPECT_EQ(CaSe->alts[1].first.constructor_name, "Nill");
    EXPECT_EQ(CaSe->alts[1].first.variables.size(), 0);
    ASSERT_EQ(CaSe->alts[1].second->get_form(), stgform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<STGLiteral*>(CaSe->alts[1].second.get())->value), 'b');
    EXPECT_EQ(CaSe->default_var, "");
    ASSERT_EQ(CaSe->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->default_expr.get())->name, "case_error");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude(
            "data List = Cons Int List | Nil\n;a l = case l of { Cons x@_ xs -> x ; Cons y ys -> j }",
            program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.at("a")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables[0], ".0");
    EXPECT_EQ(translated->bindings.at("a")->updatable, false);
    ASSERT_EQ(translated->bindings.at("a")->expr->get_form(), stgform::algebraiccase);
    CaSe = dynamic_cast<STGAlgebraicCase*>(translated->bindings.at("a")->expr.get());
    ASSERT_EQ(CaSe->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->expr.get())->name, ".0");
    ASSERT_EQ(CaSe->alts.size(), 1);
    EXPECT_EQ(CaSe->alts[0].first.constructor_name, "Cons");
    EXPECT_EQ(CaSe->alts[0].first.variables.size(), 2);
    EXPECT_EQ(CaSe->alts[0].first.variables[0], ".1");
    EXPECT_EQ(CaSe->alts[0].first.variables[1], ".2");
    ASSERT_EQ(CaSe->alts[0].second->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->alts[0].second.get())->name, ".1");
    EXPECT_EQ(CaSe->default_var, "");
    ASSERT_EQ(CaSe->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->default_expr.get())->name, "case_error");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude(
            "data List = Cons Int List | Nil\n;a l = case l of { Cons _ (Cons x xs) -> x ; Cons _ (Nil) -> 1 }",
            program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated->bindings.at("a")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables[0], ".0");
    EXPECT_EQ(translated->bindings.at("a")->updatable, false);
    ASSERT_EQ(translated->bindings.at("a")->expr->get_form(), stgform::algebraiccase);
    CaSe = dynamic_cast<STGAlgebraicCase*>(translated->bindings.at("a")->expr.get());
    ASSERT_EQ(CaSe->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->expr.get())->name, ".0");
    ASSERT_EQ(CaSe->alts.size(), 1);
    EXPECT_EQ(CaSe->alts[0].first.constructor_name, "Cons");
    EXPECT_EQ(CaSe->alts[0].first.variables.size(), 2);
    EXPECT_EQ(CaSe->alts[0].first.variables[0], ".1");
    EXPECT_EQ(CaSe->alts[0].first.variables[1], ".2");
    ASSERT_EQ(CaSe->alts[0].second->get_form(), stgform::algebraiccase);
    EXPECT_EQ(CaSe->default_var, "");
    ASSERT_EQ(CaSe->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->default_expr.get())->name, "case_error");
    CaSe = dynamic_cast<STGAlgebraicCase*>(CaSe->alts[0].second.get());
    ASSERT_EQ(CaSe->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->expr.get())->name, ".2");
    ASSERT_EQ(CaSe->alts.size(), 2);
    EXPECT_EQ(CaSe->alts[0].first.constructor_name, "Cons");
    EXPECT_EQ(CaSe->alts[0].first.variables.size(), 2);
    EXPECT_EQ(CaSe->alts[0].first.variables[0], ".3");
    EXPECT_EQ(CaSe->alts[0].first.variables[1], ".4");
    ASSERT_EQ(CaSe->alts[0].second->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->alts[0].second.get())->name, ".3");
    EXPECT_EQ(CaSe->alts[1].first.constructor_name, "Nil");
    EXPECT_EQ(CaSe->alts[1].first.variables.size(), 0);
    ASSERT_EQ(CaSe->alts[1].second->get_form(), stgform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<STGLiteral*>(CaSe->alts[1].second.get())->value), 1);
    EXPECT_EQ(CaSe->default_var, "");
    ASSERT_EQ(CaSe->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->default_expr.get())->name, "case_error");

    program = std::make_unique<Program>();
    result = parse_string_no_prelude(
            "data List = Cons Int List | Nil;"
            "data Pair = Pair List List;"
            "a p = case p of { Pair Nil ys -> let { a = 'b' } in ys ; Pair xs Nil -> xs ; Pair (Cons 1 xs) (Cons 2 ys) -> xs }",
            program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_CHAR(translated->bindings.at(".7"), 'b');
    EXPECT_EQ(translated->bindings.at("a")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("a")->argument_variables[0], ".0");
    EXPECT_EQ(translated->bindings.at("a")->updatable, false);
    ASSERT_EQ(translated->bindings.at("a")->expr->get_form(), stgform::algebraiccase);
    CaSe = dynamic_cast<STGAlgebraicCase*>(translated->bindings.at("a")->expr.get());
    ASSERT_EQ(CaSe->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->expr.get())->name, ".0");
    ASSERT_EQ(CaSe->alts.size(), 1);
    EXPECT_EQ(CaSe->alts[0].first.constructor_name, "Pair");
    EXPECT_EQ(CaSe->alts[0].first.variables.size(), 2);
    EXPECT_EQ(CaSe->alts[0].first.variables[0], ".1");
    EXPECT_EQ(CaSe->alts[0].first.variables[1], ".2");
    ASSERT_EQ(CaSe->alts[0].second->get_form(), stgform::algebraiccase);
    EXPECT_EQ(CaSe->default_var, "");
    ASSERT_EQ(CaSe->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->default_expr.get())->name, "case_error");
    CaSe = dynamic_cast<STGAlgebraicCase*>(CaSe->alts[0].second.get());
    ASSERT_EQ(CaSe->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->expr.get())->name, ".1");
    ASSERT_EQ(CaSe->alts.size(), 1);
    EXPECT_EQ(CaSe->alts[0].first.constructor_name, "Nil");
    EXPECT_EQ(CaSe->alts[0].first.variables.size(), 0);
    ASSERT_EQ(CaSe->alts[0].second->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->alts[0].second.get())->name, ".2");
    EXPECT_EQ(CaSe->default_var, "");
    ASSERT_EQ(CaSe->default_expr->get_form(), stgform::algebraiccase);
    CaSe = dynamic_cast<STGAlgebraicCase*>(CaSe->default_expr.get());
    ASSERT_EQ(CaSe->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->expr.get())->name, ".2");
    ASSERT_EQ(CaSe->alts.size(), 1);
    EXPECT_EQ(CaSe->alts[0].first.constructor_name, "Nil");
    EXPECT_EQ(CaSe->alts[0].first.variables.size(), 0);
    ASSERT_EQ(CaSe->alts[0].second->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->alts[0].second.get())->name, ".1");
    EXPECT_EQ(CaSe->default_var, "");
    ASSERT_EQ(CaSe->default_expr->get_form(), stgform::algebraiccase);
    CaSe = dynamic_cast<STGAlgebraicCase*>(CaSe->default_expr.get());
    ASSERT_EQ(CaSe->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->expr.get())->name, ".1");
    ASSERT_EQ(CaSe->alts.size(), 1);
    EXPECT_EQ(CaSe->alts[0].first.constructor_name, "Cons");
    EXPECT_EQ(CaSe->alts[0].first.variables.size(), 2);
    EXPECT_EQ(CaSe->alts[0].first.variables[0], ".3");
    EXPECT_EQ(CaSe->alts[0].first.variables[1], ".4");
    ASSERT_EQ(CaSe->alts[0].second->get_form(), stgform::literalcase);
    EXPECT_EQ(CaSe->default_var, "");
    ASSERT_EQ(CaSe->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->default_expr.get())->name, "case_error");
    cAsE = dynamic_cast<STGLiteralCase*>(CaSe->alts[0].second.get());
    ASSERT_EQ(cAsE->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->expr.get())->name, ".3");
    ASSERT_EQ(cAsE->alts.size(), 1);
    EXPECT_EQ(std::get<int>(cAsE->alts[0].first.value), 1);
    ASSERT_EQ(cAsE->alts[0].second->get_form(), stgform::algebraiccase);
    EXPECT_EQ(cAsE->default_var, "");
    ASSERT_EQ(cAsE->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->default_expr.get())->name, "case_error");
    CaSe = dynamic_cast<STGAlgebraicCase*>(cAsE->alts[0].second.get());
    ASSERT_EQ(CaSe->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->expr.get())->name, ".2");
    ASSERT_EQ(CaSe->alts.size(), 1);
    EXPECT_EQ(CaSe->alts[0].first.constructor_name, "Cons");
    EXPECT_EQ(CaSe->alts[0].first.variables.size(), 2);
    EXPECT_EQ(CaSe->alts[0].first.variables[0], ".5");
    EXPECT_EQ(CaSe->alts[0].first.variables[1], ".6");
    ASSERT_EQ(CaSe->alts[0].second->get_form(), stgform::literalcase);
    EXPECT_EQ(CaSe->default_var, "");
    ASSERT_EQ(CaSe->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(CaSe->default_expr.get())->name, "case_error");
    cAsE = dynamic_cast<STGLiteralCase*>(CaSe->alts[0].second.get());
    ASSERT_EQ(cAsE->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->expr.get())->name, ".5");
    ASSERT_EQ(cAsE->alts.size(), 1);
    EXPECT_EQ(std::get<int>(cAsE->alts[0].first.value), 2);
    ASSERT_EQ(cAsE->alts[0].second->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->alts[0].second.get())->name, ".4");
    EXPECT_EQ(cAsE->default_var, "");
    ASSERT_EQ(cAsE->default_expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(cAsE->default_expr.get())->name, "case_error");
}
