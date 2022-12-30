#include <gtest/gtest.h>
#include "test/test_utilities.hpp"
#include "stg/stg.hpp"

TEST(STGTranslation, TranslatesVariables) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string("b = a", program.get());
    ASSERT_EQ(result, 0);
    std::map<std::string, std::unique_ptr<STGLambdaForm>> translated = translate(program);
    EXPECT_EQ(translated.size(), 1);
    ASSERT_EQ(translated["b"]->free_variables.size(), 0);
    ASSERT_EQ(translated["b"]->argument_variables.size(), 0);
    ASSERT_EQ(translated["b"]->updatable, true);
    ASSERT_EQ(translated["b"]->expr->get_form(), stgform::variable);
    ASSERT_EQ(dynamic_cast<STGVariable*>(translated["b"]->expr.get())->name, "a");
}
