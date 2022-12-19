#include <gtest/gtest.h>
#include <algorithm>
#include "types/types.hpp"
#include "parser/syntax.hpp"
#include "lexer/lexer.hpp"
#include "parser/driver.hpp"

void reset_start_condition();

int parse_string(const char* str, Program *program) {
    Driver drv;
    YY_BUFFER_STATE buffer = yy_scan_string(str);
    yy_switch_to_buffer(buffer);
    reset_start_condition();
    yy::parser parse(drv, program);
    int result = parse();
    yy_delete_buffer(buffer);
    return result;
}

TEST(Types, TypeEquality) {
    EXPECT_TRUE(same_type(
            std::make_unique<TypeConstructor>("()").get(),
            std::make_unique<TypeConstructor>("()").get()));
    EXPECT_TRUE(same_type(
            std::make_unique<UniversallyQuantifiedVariable>("a").get(),
            std::make_unique<UniversallyQuantifiedVariable>("a").get()));
    EXPECT_TRUE(same_type(
            std::make_unique<TypeApplication>(new TypeConstructor("a"), new UniversallyQuantifiedVariable("b")).get(),
            std::make_unique<TypeApplication>(new TypeConstructor("a"), new UniversallyQuantifiedVariable("b")).get()));

    EXPECT_FALSE(same_type(
            std::make_unique<TypeConstructor>("()").get(),
            std::make_unique<UniversallyQuantifiedVariable>("()").get()));
    EXPECT_FALSE(same_type(
            std::make_unique<TypeConstructor>("()").get(),
            std::make_unique<TypeConstructor>("[]").get()));
    EXPECT_FALSE(same_type(
            std::make_unique<UniversallyQuantifiedVariable>("aa").get(),
            std::make_unique<UniversallyQuantifiedVariable>("a").get()));
    EXPECT_FALSE(same_type(
            std::make_unique<TypeApplication>(new TypeConstructor("a"), new UniversallyQuantifiedVariable("b")).get(),
            std::make_unique<TypeApplication>(new TypeConstructor("c"), new UniversallyQuantifiedVariable("b")).get()));
}

TEST(Types, TypeChecking) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string("a :: ()", program.get());
    //type_check(program);
}
