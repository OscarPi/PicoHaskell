#include <gtest/gtest.h>
#include "test/test_utilities.hpp"
#include "stg/stg.hpp"

TEST(STGTranslation, TranslatesVariables) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string_no_prelude("b = a", program.get());
    ASSERT_EQ(result, 0);
    auto translated = translate(program);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.size(), 1);
    EXPECT_EQ(translated->bindings.at("b")->free_variables.count("a"), 1);
    EXPECT_EQ(translated->bindings.at("b")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at("b")->updatable, true);
    ASSERT_EQ(translated->bindings.at("b")->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(translated->bindings.at("b")->expr.get())->name, "a");
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

#define EXPECT_CONS(lambda_form, hd, tl) {                                                  \
    EXPECT_EQ((lambda_form)->free_variables.size(), 2);                                     \
    EXPECT_EQ((lambda_form)->free_variables.count((hd)), 1);                                \
    EXPECT_EQ((lambda_form)->free_variables.count((tl)), 1);                                \
    EXPECT_EQ((lambda_form)->argument_variables.size(), 0);                                 \
    EXPECT_EQ((lambda_form)->updatable, false);                                             \
    ASSERT_EQ((lambda_form)->expr->get_form(), stgform::constructor);                       \
    STGConstructor *constructor = dynamic_cast<STGConstructor*>((lambda_form)->expr.get()); \
    EXPECT_EQ(constructor->arguments.size(), 2);                                            \
    EXPECT_EQ(constructor->arguments[0], (hd));                                             \
    EXPECT_EQ(constructor->arguments[1], (tl));                                             \
    EXPECT_EQ(constructor->constructor_name, ":");                                          \
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

    program = std::make_unique<Program>();
    result = parse_string_no_prelude("b = \"ab\"", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_CONS(translated->bindings.at("b"), ".0", ".1");
    EXPECT_CHAR(translated->bindings.at(".0"), 'a');
    EXPECT_CONS(translated->bindings.at(".1"), ".2", ".3");
    EXPECT_CHAR(translated->bindings.at(".2"), 'b');
    EXPECT_EQ(translated->bindings.at(".3")->free_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at(".3")->argument_variables.size(), 0);
    EXPECT_EQ(translated->bindings.at(".3")->updatable, false);
    ASSERT_EQ(translated->bindings.at(".3")->expr->get_form(), stgform::constructor);
    STGConstructor *constructor = dynamic_cast<STGConstructor*>(translated->bindings.at(".3")->expr.get());
    EXPECT_EQ(constructor->arguments.size(), 0);
    EXPECT_EQ(constructor->constructor_name, "[]");
}

//TEST(STGTranslation, TranslatesConstructors) {
//    std::unique_ptr<Program> program = std::make_unique<Program>();
//    int result = parse_string_no_prelude("data T = Test\n;b = Test", program.get());
//    ASSERT_EQ(result, 0);
//    std::map<std::string, std::unique_ptr<STGLambdaForm>> translated = translate(program);
//    EXPECT_EQ(translated["b"]->free_variables.size(), 0);
//    EXPECT_EQ(translated["b"]->argument_variables.size(), 0);
//    EXPECT_EQ(translated["b"]->updatable, false);
//    ASSERT_EQ(translated["b"]->expr->get_form(), stgform::constructor);
//    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated["b"]->expr.get())->arguments.size(), 0);
//    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated["b"]->expr.get())->constructor_name, "Test");
//}
