#include <gtest/gtest.h>
#include "test/test_utilities.hpp"
#include "stg/stg.hpp"

TEST(STGTranslation, TranslatesVariables) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string("b = a", program.get());
    ASSERT_EQ(result, 0);
    std::map<std::string, std::unique_ptr<STGLambdaForm>> translated = translate(program);
    EXPECT_EQ(translated.size(), 1);
    EXPECT_EQ(translated["b"]->free_variables.size(), 0);
    EXPECT_EQ(translated["b"]->argument_variables.size(), 0);
    EXPECT_EQ(translated["b"]->updatable, true);
    ASSERT_EQ(translated["b"]->expr->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(translated["b"]->expr.get())->name, "a");
}

#define EXPECT_CHAR(lambda_form, c) {                                                                                \
    EXPECT_EQ((lambda_form)->free_variables.size(), 0);                                                              \
    EXPECT_EQ((lambda_form)->argument_variables.size(), 0);                                                          \
    EXPECT_EQ((lambda_form)->updatable, false);                                                                      \
    ASSERT_EQ((lambda_form)->expr->get_form(), stgform::constructor);                                                \
    EXPECT_EQ(dynamic_cast<STGConstructor*>((lambda_form)->expr.get())->constructor_name, "MakeChar");               \
    ASSERT_EQ(dynamic_cast<STGConstructor*>((lambda_form)->expr.get())->arguments.size(), 1);                        \
    ASSERT_EQ(dynamic_cast<STGConstructor*>((lambda_form)->expr.get())->arguments[0]->get_form(), stgform::literal); \
    EXPECT_EQ(                                                                                                       \
            std::get<char>(                                                                                          \
                    dynamic_cast<STGLiteral*>(                                                                       \
                            dynamic_cast<STGConstructor*>((lambda_form)->expr.get())->arguments[0].get())->value),   \
            (c));                                                                                                    \
}

TEST(STGTranslation, TranslatesLiterals) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string("b = 1", program.get());
    ASSERT_EQ(result, 0);
    std::map<std::string, std::unique_ptr<STGLambdaForm>> translated = translate(program);
    EXPECT_EQ(translated.size(), 1);
    EXPECT_EQ(translated["b"]->free_variables.size(), 0);
    EXPECT_EQ(translated["b"]->argument_variables.size(), 0);
    EXPECT_EQ(translated["b"]->updatable, false);
    ASSERT_EQ(translated["b"]->expr->get_form(), stgform::constructor);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated["b"]->expr.get())->constructor_name, "MakeInt");
    ASSERT_EQ(dynamic_cast<STGConstructor*>(translated["b"]->expr.get())->arguments.size(), 1);
    ASSERT_EQ(dynamic_cast<STGConstructor*>(translated["b"]->expr.get())->arguments[0]->get_form(), stgform::literal);
    EXPECT_EQ(
            std::get<int>(
                    dynamic_cast<STGLiteral*>(
                            dynamic_cast<STGConstructor*>(translated["b"]->expr.get())->arguments[0].get())->value),
            1);

    program = std::make_unique<Program>();
    result = parse_string("b = 'a'", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated.size(), 1);
    EXPECT_CHAR(translated["b"], 'a');

    program = std::make_unique<Program>();
    result = parse_string("b = \"ab\"", program.get());
    ASSERT_EQ(result, 0);
    translated = translate(program);
    EXPECT_EQ(translated.size(), 1);
    EXPECT_EQ(translated["b"]->free_variables.size(), 0);
    EXPECT_EQ(translated["b"]->argument_variables.size(), 0);
    EXPECT_EQ(translated["b"]->updatable, true);
    ASSERT_EQ(translated["b"]->expr->get_form(), stgform::let);
    STGLet *let = dynamic_cast<STGLet*>(translated["b"]->expr.get());
    EXPECT_EQ(let->recursive, false);
    EXPECT_EQ(let->bindings.size(), 1);
    EXPECT_EQ(let->bindings.at("#t1")->free_variables.size(), 0);
    EXPECT_EQ(let->bindings.at("#t1")->argument_variables.size(), 0);
    EXPECT_EQ(let->bindings.at("#t1")->updatable, false);
    ASSERT_EQ(let->bindings.at("#t1")->expr->get_form(), stgform::constructor);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(let->bindings.at("#t1")->expr.get())->arguments.size(), 0);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(let->bindings.at("#t1")->expr.get())->constructor_name, "[]");
    ASSERT_EQ(let->expr->get_form(), stgform::let);
    let = dynamic_cast<STGLet*>(let->expr.get());
    EXPECT_EQ(let->recursive, false);
    EXPECT_EQ(let->bindings.size(), 1);
    EXPECT_CHAR(let->bindings.at("#c1"), 'b');
    ASSERT_EQ(let->expr->get_form(), stgform::let);
    let = dynamic_cast<STGLet*>(let->expr.get());
    EXPECT_EQ(let->recursive, false);
    EXPECT_EQ(let->bindings.size(), 1);
    EXPECT_EQ(let->bindings.at("#t0")->free_variables.size(), 2);
    EXPECT_EQ(let->bindings.at("#t0")->free_variables[0], "#c1");
    EXPECT_EQ(let->bindings.at("#t0")->free_variables[1], "#t1");
    EXPECT_EQ(let->bindings.at("#t0")->argument_variables.size(), 0);
    EXPECT_EQ(let->bindings.at("#t0")->updatable, false);
    ASSERT_EQ(let->bindings.at("#t0")->expr->get_form(), stgform::constructor);
    STGConstructor *constructor = dynamic_cast<STGConstructor*>(let->bindings.at("#t0")->expr.get());
    EXPECT_EQ(constructor->arguments.size(), 2);
    ASSERT_EQ(constructor->arguments[0]->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(constructor->arguments[0].get())->name, "#c1");
    ASSERT_EQ(constructor->arguments[1]->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(constructor->arguments[1].get())->name, "#t1");
    EXPECT_EQ(constructor->constructor_name, ":");
    ASSERT_EQ(let->expr->get_form(), stgform::let);
    let = dynamic_cast<STGLet*>(let->expr.get());
    EXPECT_EQ(let->recursive, false);
    EXPECT_EQ(let->bindings.size(), 1);
    EXPECT_CHAR(let->bindings.at("#c0"), 'a');
    ASSERT_EQ(let->expr->get_form(), stgform::constructor);
    constructor = dynamic_cast<STGConstructor*>(let->expr.get());
    EXPECT_EQ(constructor->arguments.size(), 2);
    ASSERT_EQ(constructor->arguments[0]->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(constructor->arguments[0].get())->name, "#c0");
    ASSERT_EQ(constructor->arguments[1]->get_form(), stgform::variable);
    EXPECT_EQ(dynamic_cast<STGVariable*>(constructor->arguments[1].get())->name, "#t0");
    EXPECT_EQ(constructor->constructor_name, ":");
}

TEST(STGTranslation, TranslatesConstructors) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string("data T = Test\n;b = Test", program.get());
    ASSERT_EQ(result, 0);
    std::map<std::string, std::unique_ptr<STGLambdaForm>> translated = translate(program);
    EXPECT_EQ(translated.size(), 1);
    EXPECT_EQ(translated["b"]->free_variables.size(), 0);
    EXPECT_EQ(translated["b"]->argument_variables.size(), 0);
    EXPECT_EQ(translated["b"]->updatable, false);
    ASSERT_EQ(translated["b"]->expr->get_form(), stgform::constructor);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated["b"]->expr.get())->arguments.size(), 0);
    EXPECT_EQ(dynamic_cast<STGConstructor*>(translated["b"]->expr.get())->constructor_name, "Test");
}
