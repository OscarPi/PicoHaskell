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
    ASSERT_NE(program->typeSignatures["a"], nullptr);
    EXPECT_TRUE(sameType(program->typeSignatures["a"], tUnit));

    program = std::make_shared<Program>();
    result = parse_string("a :: [] Int", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->typeSignatures["a"], nullptr);
    EXPECT_TRUE(sameType_ignoreKinds(program->typeSignatures["a"], makeListType(tInt)));

    program = std::make_shared<Program>();
    result = parse_string("a :: [Int]", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->typeSignatures["a"], nullptr);
    EXPECT_TRUE(sameType_ignoreKinds(program->typeSignatures["a"], makeListType(tInt)));

    program = std::make_shared<Program>();
    result = parse_string("a :: Int -> Int -> Int", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->typeSignatures["a"], nullptr);
    type expected = makeFunctionType(tInt, makeFunctionType(tInt, tInt));
    EXPECT_TRUE(sameType_ignoreKinds(program->typeSignatures["a"], expected));

    program = std::make_shared<Program>();
    result = parse_string("a :: (->) Int (Int -> Int)", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->typeSignatures["a"], nullptr);
    expected = makeFunctionType(tInt, makeFunctionType(tInt, tInt));
    EXPECT_TRUE(sameType_ignoreKinds(program->typeSignatures["a"], expected));

    program = std::make_shared<Program>();
    result = parse_string("a :: (Int -> Int) -> Int", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->typeSignatures["a"], nullptr);
    expected = makeFunctionType(makeFunctionType(tInt, tInt), tInt);
    EXPECT_TRUE(sameType_ignoreKinds(program->typeSignatures["a"], expected));

    program = std::make_shared<Program>();
    result = parse_string("a :: cheesecake -> cheesecake", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->typeSignatures["a"], nullptr);
    type cheesecake = std::make_shared<const TypeVariable>("cheesecake", nullptr);
    expected = makeFunctionType(cheesecake, cheesecake);
    EXPECT_TRUE(sameType_ignoreKinds(program->typeSignatures["a"], expected));

    program = std::make_shared<Program>();
    result = parse_string("a :: (Int,Double)", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->typeSignatures["a"], nullptr);
    EXPECT_TRUE(sameType_ignoreKinds(program->typeSignatures["a"], makePairType(tInt, tDouble)));

    program = std::make_shared<Program>();
    result = parse_string("a :: (Int,Double,Int)", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->typeSignatures["a"], nullptr);
    EXPECT_TRUE(sameType_ignoreKinds(program->typeSignatures["a"], makeTupleType({tInt, tDouble, tInt})));

    program = std::make_shared<Program>();
    result = parse_string("a :: (,,) Int Double Int", program);
    ASSERT_EQ(result, 0);
    ASSERT_NE(program->typeSignatures["a"], nullptr);
    EXPECT_TRUE(sameType_ignoreKinds(program->typeSignatures["a"], makeTupleType({tInt, tDouble, tInt})));
}

TEST(Parser, ParsesDataDecls) {
    std::shared_ptr<Program> program = std::make_shared<Program>();
    int result = parse_string("data hey", program);
    ASSERT_EQ(result, 1);

    program = std::make_shared<Program>();
    result = parse_string("data Hi", program);
    ASSERT_EQ(result, 0);
    auto tConstructor = program->typeConstructors["Hi"];
    ASSERT_NE(tConstructor, nullptr);
    EXPECT_EQ(tConstructor->getLineNo(), 1);
    EXPECT_EQ(tConstructor->getName(), "Hi");
    EXPECT_EQ(tConstructor->getArgumentVariables().size(), 0);
    EXPECT_EQ(tConstructor->getDataConstructors().size(), 0);

    program = std::make_shared<Program>();
    EXPECT_THROW(parse_string("data Hi a a", program), ParseError);

    program = std::make_shared<Program>();
    EXPECT_THROW(parse_string("data Hi a = Bye b", program), ParseError);

    program = std::make_shared<Program>();
    result = parse_string("data Hi = Bye | No", program);
    ASSERT_EQ(result, 0);
    tConstructor = program->typeConstructors["Hi"];
    ASSERT_NE(tConstructor, nullptr);
    EXPECT_EQ(tConstructor->getLineNo(), 1);
    EXPECT_EQ(tConstructor->getName(), "Hi");
    EXPECT_EQ(tConstructor->getArgumentVariables().size(), 0);
    ASSERT_EQ(tConstructor->getDataConstructors().size(), 2);
    auto dConstructor = tConstructor->getDataConstructors()[0];
    EXPECT_EQ(dConstructor->getLineNo(), 1);
    EXPECT_EQ(dConstructor->getName(), "Bye");
    EXPECT_EQ(dConstructor->getTypes().size(), 0);
    EXPECT_EQ(dConstructor->getTConstructor(), tConstructor);
    dConstructor = tConstructor->getDataConstructors()[1];
    EXPECT_EQ(dConstructor->getLineNo(), 1);
    EXPECT_EQ(dConstructor->getName(), "No");
    EXPECT_EQ(dConstructor->getTypes().size(), 0);
    EXPECT_EQ(dConstructor->getTConstructor(), tConstructor);

    program = std::make_shared<Program>();
    result = parse_string("data Hi a b = Bye a | No", program);
    ASSERT_EQ(result, 0);
    tConstructor = program->typeConstructors["Hi"];
    ASSERT_NE(tConstructor, nullptr);
    EXPECT_EQ(tConstructor->getLineNo(), 1);
    EXPECT_EQ(tConstructor->getName(), "Hi");
    EXPECT_EQ(tConstructor->getArgumentVariables().size(), 2);
    ASSERT_EQ(tConstructor->getDataConstructors().size(), 2);
    dConstructor = tConstructor->getDataConstructors()[0];
    EXPECT_EQ(dConstructor->getLineNo(), 1);
    EXPECT_EQ(dConstructor->getName(), "Bye");
    EXPECT_EQ(dConstructor->getTypes().size(), 1);
    EXPECT_EQ(dConstructor->getTConstructor(), tConstructor);
    dConstructor = tConstructor->getDataConstructors()[1];
    EXPECT_EQ(dConstructor->getLineNo(), 1);
    EXPECT_EQ(dConstructor->getName(), "No");
    EXPECT_EQ(dConstructor->getTypes().size(), 0);
    EXPECT_EQ(dConstructor->getTConstructor(), tConstructor);

    program = std::make_shared<Program>();
    EXPECT_THROW(parse_string("data Hi a\n;data Hi", program), ParseError);

    program = std::make_shared<Program>();
    EXPECT_THROW(parse_string("data Hi = One\n;data Bye = One", program), ParseError);

    program = std::make_shared<Program>();
    result = parse_string("data Hi = Bye\n;data Bye = Hi", program);
    ASSERT_EQ(result, 0);
    tConstructor = program->typeConstructors["Hi"];
    ASSERT_NE(tConstructor, nullptr);
    EXPECT_EQ(tConstructor->getLineNo(), 1);
    EXPECT_EQ(tConstructor->getName(), "Hi");
    EXPECT_EQ(tConstructor->getArgumentVariables().size(), 0);
    ASSERT_EQ(tConstructor->getDataConstructors().size(), 1);
    dConstructor = tConstructor->getDataConstructors()[0];
    EXPECT_EQ(dConstructor->getLineNo(), 1);
    EXPECT_EQ(dConstructor->getName(), "Bye");
    EXPECT_EQ(dConstructor->getTypes().size(), 0);
    EXPECT_EQ(dConstructor->getTConstructor(), tConstructor);
    tConstructor = program->typeConstructors["Bye"];
    ASSERT_NE(tConstructor, nullptr);
    EXPECT_EQ(tConstructor->getLineNo(), 2);
    EXPECT_EQ(tConstructor->getName(), "Bye");
    EXPECT_EQ(tConstructor->getArgumentVariables().size(), 0);
    ASSERT_EQ(tConstructor->getDataConstructors().size(), 1);
    dConstructor = tConstructor->getDataConstructors()[0];
    EXPECT_EQ(dConstructor->getLineNo(), 2);
    EXPECT_EQ(dConstructor->getName(), "Hi");
    EXPECT_EQ(dConstructor->getTypes().size(), 0);
    EXPECT_EQ(dConstructor->getTConstructor(), tConstructor);
}

TEST(Parser, ParsesVariableExpressions) {
    auto program = std::make_shared<Program>();
    auto result = parse_string("a = b \n;c = d", program);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(program->bindings.size(), 2);
    ASSERT_NE(program->bindings["a"], nullptr);
    auto a = program->bindings["a"];
    ASSERT_EQ(a->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(a)->name, "b");
    ASSERT_NE(program->bindings["c"], nullptr);
    auto c = program->bindings["c"];
    ASSERT_EQ(c->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(c)->name, "d");

    program = std::make_shared<Program>();
    EXPECT_THROW(parse_string("a = b \n;a = d", program), ParseError);
}

TEST(Parser, ParsesLiteralExpressions) {
    auto program = std::make_shared<Program>();
    auto result = parse_string("a = 1\n;b = 'a'\n;c = \"hi\"", program);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(program->bindings.size(), 3);

    ASSERT_NE(program->bindings["a"], nullptr);
    auto a = program->bindings["a"];
    ASSERT_EQ(a->getForm(), expform::lit);
    ASSERT_TRUE(std::holds_alternative<int>(std::dynamic_pointer_cast<Literal>(a)->value));
    EXPECT_EQ(std::get<int>(std::dynamic_pointer_cast<Literal>(a)->value), 1);

    ASSERT_NE(program->bindings["b"], nullptr);
    auto b = program->bindings["b"];
    ASSERT_EQ(b->getForm(), expform::lit);
    ASSERT_TRUE(std::holds_alternative<char>(std::dynamic_pointer_cast<Literal>(b)->value));
    EXPECT_EQ(std::get<char>(std::dynamic_pointer_cast<Literal>(b)->value), 'a');

    ASSERT_NE(program->bindings["c"], nullptr);
    auto c = program->bindings["c"];
    ASSERT_EQ(c->getForm(), expform::lit);
    ASSERT_TRUE(std::holds_alternative<std::string>(std::dynamic_pointer_cast<Literal>(c)->value));
    EXPECT_EQ(std::get<std::string>(std::dynamic_pointer_cast<Literal>(c)->value), "hi");
}

TEST(Parser, ParsesConstructorExpressions) {
    auto program = std::make_shared<Program>();
    auto result = parse_string("a = ()\n;b = []\n;c = (,,)\n;d = Toast", program);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(program->bindings.size(), 4);

    ASSERT_NE(program->bindings["a"], nullptr);
    auto a = program->bindings["a"];
    ASSERT_EQ(a->getForm(), expform::con);
    EXPECT_EQ(std::dynamic_pointer_cast<Constructor>(a)->name, "()");

    ASSERT_NE(program->bindings["b"], nullptr);
    auto b = program->bindings["b"];
    ASSERT_EQ(b->getForm(), expform::con);
    EXPECT_EQ(std::dynamic_pointer_cast<Constructor>(b)->name, "[]");

    ASSERT_NE(program->bindings["c"], nullptr);
    auto c = program->bindings["c"];
    ASSERT_EQ(c->getForm(), expform::con);
    EXPECT_EQ(std::dynamic_pointer_cast<Constructor>(c)->name, "(,,)");

    ASSERT_NE(program->bindings["d"], nullptr);
    auto d = program->bindings["d"];
    ASSERT_EQ(d->getForm(), expform::con);
    EXPECT_EQ(std::dynamic_pointer_cast<Constructor>(d)->name, "Toast");
}

TEST(Parser, ParsesApplicationExpressions) {
    auto program = std::make_shared<Program>();
    auto result = parse_string("a = b c d", program);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(program->bindings.size(), 1);

    ASSERT_NE(program->bindings["a"], nullptr);
    ASSERT_EQ(program->bindings["a"]->getForm(), expform::app);
    auto a = std::dynamic_pointer_cast<Application>(program->bindings["a"]);
    ASSERT_EQ(a->right->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(a->right)->name, "d");
    ASSERT_EQ(a->left->getForm(), expform::app);
    auto l = std::dynamic_pointer_cast<Application>(a->left);
    ASSERT_EQ(l->left->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(l->left)->name, "b");
    ASSERT_EQ(l->right->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(l->right)->name, "c");
}

TEST(Parser, ParsesLambdaAbstractions) {
    auto program = std::make_shared<Program>();
    auto result = parse_string("l = \\ a b -> a", program);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(program->bindings.size(), 1);

    ASSERT_NE(program->bindings["l"], nullptr);
    ASSERT_EQ(program->bindings["l"]->getForm(), expform::lam);
    auto l = std::dynamic_pointer_cast<Lambda>(program->bindings["l"]);
    auto args = l->args;
    EXPECT_EQ(args.size(), 2);
    EXPECT_EQ(std::count(args.begin(), args.end(), "a"), 1);
    EXPECT_EQ(std::count(args.begin(), args.end(), "b"), 1);
    ASSERT_EQ(l->body->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(l->body)->name, "a");
}

#define TESTINFIXOP(opstr, oper) { \
    auto program = std::make_shared<Program>();                            \
    auto result = parse_string("a = b " opstr " c", program);              \
    ASSERT_EQ(result, 0);                                                  \
    EXPECT_EQ(program->bindings.size(), 1);                                \
    ASSERT_NE(program->bindings["a"], nullptr);                            \
    ASSERT_EQ(program->bindings["a"]->getForm(), expform::bop);            \
    auto a = std::dynamic_pointer_cast<BuiltInOp>(program->bindings["a"]); \
    ASSERT_EQ(a->left->getForm(), expform::var);                           \
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(a->left)->name, "b");    \
    ASSERT_EQ(a->right->getForm(), expform::var);                          \
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(a->right)->name, "c");   \
    EXPECT_EQ(a->op, (oper));                                              \
}

TEST(Parser, ParsesInfix) {
    TESTINFIXOP("+", builtinop::add);
    TESTINFIXOP("-", builtinop::subtract);
    TESTINFIXOP("*", builtinop::times);
    TESTINFIXOP("/", builtinop::divide);
    TESTINFIXOP("==", builtinop::equality);
    TESTINFIXOP("/=", builtinop::inequality);
    TESTINFIXOP("<", builtinop::lt);
    TESTINFIXOP("<=", builtinop::lte);
    TESTINFIXOP(">", builtinop::gt);
    TESTINFIXOP(">=", builtinop::gte);
    TESTINFIXOP("&&", builtinop::land);
    TESTINFIXOP("||", builtinop::lor);

    auto program = std::make_shared<Program>();
    auto result = parse_string("a = -c", program);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(program->bindings.size(), 1);
    ASSERT_NE(program->bindings["a"], nullptr);
    ASSERT_EQ(program->bindings["a"]->getForm(), expform::bop);
    auto a = std::dynamic_pointer_cast<BuiltInOp>(program->bindings["a"]);
    EXPECT_EQ(a->left, nullptr);
    ASSERT_EQ(a->right->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(a->right)->name, "c");
    EXPECT_EQ(a->op, builtinop::negate);

    program = std::make_shared<Program>();
    result = parse_string("a = b:c", program);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(program->bindings.size(), 1);
    ASSERT_NE(program->bindings["a"], nullptr);
    ASSERT_EQ(program->bindings["a"]->getForm(), expform::app);
    auto b = std::dynamic_pointer_cast<Application>(program->bindings["a"]);
    ASSERT_EQ(b->left->getForm(), expform::app);
    auto l = std::dynamic_pointer_cast<Application>(b->left);
    ASSERT_EQ(l->left->getForm(), expform::con);
    EXPECT_EQ(std::dynamic_pointer_cast<Constructor>(l->left)->name, ":");
    ASSERT_EQ(l->right->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(l->right)->name, "b");
    ASSERT_EQ(b->right->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(b->right)->name, "c");

    program = std::make_shared<Program>();
    result = parse_string("a = b.c", program);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(program->bindings.size(), 1);
    ASSERT_NE(program->bindings["a"], nullptr);
    ASSERT_EQ(program->bindings["a"]->getForm(), expform::app);
    b = std::dynamic_pointer_cast<Application>(program->bindings["a"]);
    ASSERT_EQ(b->left->getForm(), expform::app);
    l = std::dynamic_pointer_cast<Application>(b->left);
    ASSERT_EQ(l->left->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(l->left)->name, ".");
    ASSERT_EQ(l->right->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(l->right)->name, "b");
    ASSERT_EQ(b->right->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(b->right)->name, "c");
}

TEST(Parser, ParsesConditionals) {
    auto program = std::make_shared<Program>();
    auto result = parse_string("i = if a then 1 else 2", program);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(program->bindings.size(), 1);

    ASSERT_NE(program->bindings["i"], nullptr);
    ASSERT_EQ(program->bindings["i"]->getForm(), expform::cas);
    auto i = std::dynamic_pointer_cast<Case>(program->bindings["i"]);
    ASSERT_EQ(i->exp->getForm(), expform::var);
    EXPECT_EQ(std::dynamic_pointer_cast<Variable>(i->exp)->name, "a");

    auto alts = i->alts;
    ASSERT_EQ(alts.size(), 2);
    ASSERT_EQ(alts[0].first->getForm(), patform::con);
    auto p = std::dynamic_pointer_cast<ConPattern>(alts[0].first);
    EXPECT_EQ(p->name, "True");
    EXPECT_EQ(p->args.size(), 0);
    ASSERT_EQ(alts[0].second->getForm(), expform::lit);
    auto l = std::dynamic_pointer_cast<Literal>(alts[0].second);
    EXPECT_EQ(std::get<int>(l->value), 1);

    ASSERT_EQ(alts[1].first->getForm(), patform::con);
    p = std::dynamic_pointer_cast<ConPattern>(alts[1].first);
    EXPECT_EQ(p->name, "False");
    EXPECT_EQ(p->args.size(), 0);
    ASSERT_EQ(alts[1].second->getForm(), expform::lit);
    l = std::dynamic_pointer_cast<Literal>(alts[1].second);
    EXPECT_EQ(std::get<int>(l->value), 2);
}
