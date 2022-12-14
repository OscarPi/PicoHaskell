#include <gtest/gtest.h>
#include <memory>
#include "parser/driver.hpp"
#include "lexer/lexer.hpp"

bool sameType_ignoreKinds(const type &a, const type &b) {
    std::shared_ptr<const TypeVariable> variable1;
    std::shared_ptr<const TypeVariable> variable2;
    std::shared_ptr<const TypeConstructor> constructor1;
    std::shared_ptr<const TypeConstructor> constructor2;
    std::shared_ptr<const TypeApplication> application1;
    std::shared_ptr<const TypeApplication> application2;
    if (a->getType() != b->getType()) {
        return false;
    }
    switch (a->getType()) {
        case ttype::var:
            variable1 = std::dynamic_pointer_cast<const TypeVariable>(a);
            variable2 = std::dynamic_pointer_cast<const TypeVariable>(b);
            return variable1->getId() == variable2->getId();
        case ttype::con:
            constructor1 = std::dynamic_pointer_cast<const TypeConstructor>(a);
            constructor2 = std::dynamic_pointer_cast<const TypeConstructor>(b);
            return constructor1->getId() == constructor2->getId();
        case ttype::ap:
            application1 = std::dynamic_pointer_cast<const TypeApplication>(a);
            application2 = std::dynamic_pointer_cast<const TypeApplication>(b);
            return sameType_ignoreKinds(application1->getLeft(), application2->getLeft()) && sameType_ignoreKinds(application1->getRight(), application2->getRight());
        case ttype::gen:
            return std::dynamic_pointer_cast<const TypeGeneric>(a)->getN() == std::dynamic_pointer_cast<const TypeGeneric>(b)->getN();
    }
}

void reset_start_condition();

int parse_string(const char* str, const std::shared_ptr<Program> &program) {
    Driver drv;
    YY_BUFFER_STATE buffer = yy_scan_string(str);
    yy_switch_to_buffer(buffer);
    reset_start_condition();
    yy::parser parse(drv, program);
    int result = parse();
    yy_delete_buffer(buffer);
    return result;
}

TEST(Parser, ParsesTypeSignatures) {
    std::shared_ptr<Program> program = std::make_shared<Program>();
    int result = parse_string("a :: ()", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->getTypeSignature("a"), nullptr);
    EXPECT_TRUE(sameType(program->getTypeSignature("a"), tUnit));

    program = std::make_shared<Program>();
    result = parse_string("a :: [] Int", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->getTypeSignature("a"), nullptr);
    EXPECT_TRUE(sameType_ignoreKinds(program->getTypeSignature("a"), makeListType(tInt)));

    program = std::make_shared<Program>();
    result = parse_string("a :: [Int]", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->getTypeSignature("a"), nullptr);
    EXPECT_TRUE(sameType_ignoreKinds(program->getTypeSignature("a"), makeListType(tInt)));

    program = std::make_shared<Program>();
    result = parse_string("a :: Int -> Int -> Int", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->getTypeSignature("a"), nullptr);
    type expected = makeFunctionType(tInt, makeFunctionType(tInt, tInt));
    EXPECT_TRUE(sameType_ignoreKinds(program->getTypeSignature("a"), expected));

    program = std::make_shared<Program>();
    result = parse_string("a :: (->) Int (Int -> Int)", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->getTypeSignature("a"), nullptr);
    expected = makeFunctionType(tInt, makeFunctionType(tInt, tInt));
    EXPECT_TRUE(sameType_ignoreKinds(program->getTypeSignature("a"), expected));

    program = std::make_shared<Program>();
    result = parse_string("a :: (Int -> Int) -> Int", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->getTypeSignature("a"), nullptr);
    expected = makeFunctionType(makeFunctionType(tInt, tInt), tInt);
    EXPECT_TRUE(sameType_ignoreKinds(program->getTypeSignature("a"), expected));

    program = std::make_shared<Program>();
    result = parse_string("a :: cheesecake -> cheesecake", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->getTypeSignature("a"), nullptr);
    type cheesecake = std::make_shared<const TypeVariable>("cheesecake", nullptr);
    expected = makeFunctionType(cheesecake, cheesecake);
    EXPECT_TRUE(sameType_ignoreKinds(program->getTypeSignature("a"), expected));

    program = std::make_shared<Program>();
    result = parse_string("a :: (Int,Double)", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->getTypeSignature("a"), nullptr);
    EXPECT_TRUE(sameType_ignoreKinds(program->getTypeSignature("a"), makePairType(tInt, tDouble)));

    program = std::make_shared<Program>();
    result = parse_string("a :: (Int,Double,Int)", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->getTypeSignature("a"), nullptr);
    EXPECT_TRUE(sameType_ignoreKinds(program->getTypeSignature("a"), makeTupleType({tInt, tDouble, tInt})));

    program = std::make_shared<Program>();
    result = parse_string("a :: (,,) Int Double Int", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->getTypeSignature("a"), nullptr);
    EXPECT_TRUE(sameType_ignoreKinds(program->getTypeSignature("a"), makeTupleType({tInt, tDouble, tInt})));
}
