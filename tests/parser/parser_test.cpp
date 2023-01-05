#include <gtest/gtest.h>
#include <memory>
#include "test/test_utilities.hpp"

TEST(Parser, ParsesTypeSignatures) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string("a :: ()", program.get());
    ASSERT_EQ(result, 0);
    EXPECT_TRUE(
            same_type(program->type_signatures["a"].get(),
            std::make_unique<TypeConstructor>("()").get()));

    program = std::make_unique<Program>();
    result = parse_string("a :: [] Int", program.get());
    ASSERT_EQ(result, 0);
    EXPECT_TRUE(same_type(program->type_signatures["a"].get(), make_list_type(new TypeConstructor("Int"))));

    program = std::make_unique<Program>();
    result = parse_string("a :: [Int]", program.get());
    ASSERT_EQ(result, 0);
    EXPECT_TRUE(same_type(program->type_signatures["a"].get(), make_list_type(new TypeConstructor("Int"))));

    program = std::make_unique<Program>();
    result = parse_string("a :: Int -> Int -> Int", program.get());
    ASSERT_EQ(result, 0);
    auto expected = make_function_type(new TypeConstructor("Int"), make_function_type(new TypeConstructor("Int"), new TypeConstructor("Int")));
    EXPECT_TRUE(same_type(program->type_signatures["a"].get(), expected));

    program = std::make_unique<Program>();
    result = parse_string("a :: (->) Int (Int -> Int)", program.get());
    ASSERT_EQ(result, 0);
    expected = make_function_type(new TypeConstructor("Int"), make_function_type(new TypeConstructor("Int"), new TypeConstructor("Int")));
    EXPECT_TRUE(same_type(program->type_signatures["a"].get(), expected));

    program = std::make_unique<Program>();
    result = parse_string("a :: (Int -> Int) -> Int", program.get());
    ASSERT_EQ(result, 0);
    expected = make_function_type(make_function_type(new TypeConstructor("Int"), new TypeConstructor("Int")), new TypeConstructor("Int"));
    EXPECT_TRUE(same_type(program->type_signatures["a"].get(), expected));

    program = std::make_unique<Program>();
    result = parse_string("a :: cheesecake -> cheesecake", program.get());
    ASSERT_EQ(result, 0);
    auto cheesecake = new UniversallyQuantifiedVariable("cheesecake");
    expected = make_function_type(cheesecake, cheesecake);
    EXPECT_TRUE(same_type(program->type_signatures["a"].get(), expected));

    program = std::make_unique<Program>();
    result = parse_string("a :: (Int,Double)", program.get());
    ASSERT_EQ(result, 0);
    EXPECT_TRUE(same_type(program->type_signatures["a"].get(), make_tuple_type({new TypeConstructor("Int"), new TypeConstructor("Double")})));

    program = std::make_unique<Program>();
    result = parse_string("a :: (Int,Double,Int)", program.get());
    ASSERT_EQ(result, 0);
    EXPECT_TRUE(same_type(program->type_signatures["a"].get(), make_tuple_type({new TypeConstructor("Int"), new TypeConstructor("Double"), new TypeConstructor("Int")})));

    program = std::make_unique<Program>();
    result = parse_string("a :: (,,) Int Double Int", program.get());
    ASSERT_EQ(result, 0);
    EXPECT_TRUE(same_type(program->type_signatures["a"].get(), make_tuple_type({new TypeConstructor("Int"), new TypeConstructor("Double"), new TypeConstructor("Int")})));
}

TEST(Parser, ParsesDataDecls) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string("data hey", program.get());
    ASSERT_EQ(result, 1);

    program = std::make_unique<Program>();
    result = parse_string("data Hi", program.get());
    ASSERT_EQ(result, 0);
    const auto &hi = program->type_constructors["Hi"];

    EXPECT_EQ(hi->line, 1);
    EXPECT_EQ(hi->name, "Hi");
    EXPECT_EQ(hi->argument_variables.size(), 0);
    EXPECT_EQ(hi->data_constructors.size(), 0);

    program = std::make_unique<Program>();
    result = parse_string("data Hi = Bye | No", program.get());
    ASSERT_EQ(result, 0);
    const auto &hi2 = program->type_constructors["Hi"];

    EXPECT_EQ(hi2->line, 1);
    EXPECT_EQ(hi2->name, "Hi");
    EXPECT_EQ(hi2->argument_variables.size(), 0);
    ASSERT_EQ(hi2->data_constructors.size(), 2);
    const auto &bye = program->data_constructors["Bye"];
    EXPECT_EQ(bye->line, 1);
    EXPECT_EQ(bye->name, "Bye");
    EXPECT_EQ(bye->types.size(), 0);
    EXPECT_EQ(bye->type_constructor, "Hi");
    const auto &no = program->data_constructors["No"];
    EXPECT_EQ(no->line, 1);
    EXPECT_EQ(no->name, "No");
    EXPECT_EQ(no->types.size(), 0);
    EXPECT_EQ(no->type_constructor, "Hi");

    program = std::make_unique<Program>();
    result = parse_string("data Hi a b = Bye a | No", program.get());
    ASSERT_EQ(result, 0);
    const auto &hi3 = program->type_constructors["Hi"];

    EXPECT_EQ(hi3->line, 1);
    EXPECT_EQ(hi3->name, "Hi");
    EXPECT_EQ(hi3->argument_variables.size(), 2);
    ASSERT_EQ(hi3->data_constructors.size(), 2);
    const auto &bye2 = program->data_constructors["Bye"];
    EXPECT_EQ(bye2->line, 1);
    EXPECT_EQ(bye2->name, "Bye");
    EXPECT_EQ(bye2->types.size(), 1);
    EXPECT_EQ(bye2->type_constructor, "Hi");
    const auto &no2 = program->data_constructors["No"];
    EXPECT_EQ(no2->line, 1);
    EXPECT_EQ(no2->name, "No");
    EXPECT_EQ(no2->types.size(), 0);
    EXPECT_EQ(no2->type_constructor, "Hi");

    program = std::make_unique<Program>();
    EXPECT_THROW(parse_string("data Hi a\n;data Hi", program.get()), ParseError);

    program = std::make_unique<Program>();
    EXPECT_THROW(parse_string("data Hi = One\n;data Bye = One", program.get()), ParseError);

    program = std::make_unique<Program>();
    EXPECT_THROW(parse_string("data Hi a a = One", program.get()), ParseError);

    program = std::make_unique<Program>();
    result = parse_string("data Hi = Bye\n;data Bye = Hi", program.get());
    ASSERT_EQ(result, 0);
    const auto &hi4 = program->type_constructors["Hi"];

    EXPECT_EQ(hi4->line, 1);
    EXPECT_EQ(hi4->name, "Hi");
    EXPECT_EQ(hi4->argument_variables.size(), 0);
    ASSERT_EQ(hi4->data_constructors.size(), 1);
    const auto &bye3 = program->data_constructors["Bye"];
    EXPECT_EQ(bye3->line, 1);
    EXPECT_EQ(bye3->name, "Bye");
    EXPECT_EQ(bye3->types.size(), 0);
    EXPECT_EQ(bye3->type_constructor, "Hi");
    const auto &bye4 = program->type_constructors["Bye"];

    EXPECT_EQ(bye4->line, 2);
    EXPECT_EQ(bye4->name, "Bye");
    EXPECT_EQ(bye4->argument_variables.size(), 0);
    ASSERT_EQ(bye4->data_constructors.size(), 1);
    const auto &hi5 = program->data_constructors["Hi"];
    EXPECT_EQ(hi5->line, 2);
    EXPECT_EQ(hi5->name, "Hi");
    EXPECT_EQ(hi5->types.size(), 0);
    EXPECT_EQ(hi5->type_constructor, "Bye");
}

TEST(Parser, ParsesVariableExpressions) {
    auto program = std::make_unique<Program>();
    auto result = parse_string("a = b \n;c = d", program.get());
    ASSERT_EQ(result, 0);

    const auto &a = program->bindings["a"];
    EXPECT_EQ(dynamic_cast<Variable*>(a.get())->name, "b");

    const auto &c = program->bindings["c"];
    ASSERT_EQ(c->get_form(), expform::variable);
    EXPECT_EQ(dynamic_cast<Variable*>(c.get())->name, "d");

    program = std::make_unique<Program>();
    EXPECT_THROW(parse_string("a = b \n;a = d", program.get()), ParseError);
}

TEST(Parser, ParsesLiteralExpressions) {
    auto program = std::make_unique<Program>();
    auto result = parse_string("a = 1\n;b = 'a'\n;c = \"hi\"", program.get());
    ASSERT_EQ(result, 0);

    const auto &a = program->bindings["a"];
    EXPECT_EQ(std::get<int>(dynamic_cast<Literal*>(a.get())->value), 1);

    const auto &b = program->bindings["b"];
    EXPECT_EQ(std::get<char>(dynamic_cast<Literal*>(b.get())->value), 'a');

    auto str = dynamic_cast<Application*>(program->bindings["c"].get());
    ASSERT_EQ(str->left->get_form(), expform::application);
    auto l = dynamic_cast<Application*>(str->left.get());
    ASSERT_EQ(l->left->get_form(), expform::constructor);
    EXPECT_EQ(dynamic_cast<Constructor*>(l->left.get())->name, ":");
    ASSERT_EQ(l->right->get_form(), expform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<Literal*>(l->right.get())->value), 'h');
    ASSERT_EQ(str->right->get_form(), expform::application);
    auto r = dynamic_cast<Application*>(str->right.get());
    ASSERT_EQ(r->left->get_form(), expform::application);
    auto rl = dynamic_cast<Application*>(r->left.get());
    ASSERT_EQ(rl->left->get_form(), expform::constructor);
    EXPECT_EQ(dynamic_cast<Constructor*>(rl->left.get())->name, ":");
    ASSERT_EQ(rl->right->get_form(), expform::literal);
    EXPECT_EQ(std::get<char>(dynamic_cast<Literal*>(rl->right.get())->value), 'i');
    ASSERT_EQ(r->right->get_form(), expform::constructor);
    EXPECT_EQ(dynamic_cast<Constructor*>(r->right.get())->name, "[]");
}

TEST(Parser, ParsesConstructorExpressions) {
    auto program = std::make_unique<Program>();
    auto result = parse_string("a = ()\n;b = []\n;c = (,,)\n;d = Toast", program.get());
    ASSERT_EQ(result, 0);

    const auto &a = program->bindings["a"];
    EXPECT_EQ(dynamic_cast<Constructor*>(a.get())->name, "()");

    const auto &b = program->bindings["b"];
    EXPECT_EQ(dynamic_cast<Constructor*>(b.get())->name, "[]");

    const auto &c = program->bindings["c"];
    EXPECT_EQ(dynamic_cast<Constructor*>(c.get())->name, "(,,)");

    const auto &d = program->bindings["d"];
    EXPECT_EQ(dynamic_cast<Constructor*>(d.get())->name, "Toast");
}

TEST(Parser, ParsesApplicationExpressions) {
    auto program = std::make_unique<Program>();
    auto result = parse_string("a = b c d", program.get());
    ASSERT_EQ(result, 0);

    const auto &a = dynamic_cast<Application*>(program->bindings["a"].get());
    EXPECT_EQ(dynamic_cast<Variable*>(a->right.get())->name, "d");
    auto l = dynamic_cast<Application*>(a->left.get());
    EXPECT_EQ(dynamic_cast<Variable*>(l->left.get())->name, "b");
    EXPECT_EQ(dynamic_cast<Variable*>(l->right.get())->name, "c");
}

TEST(Parser, ParsesLambdaAbstractions) {
    auto program = std::make_unique<Program>();
    auto result = parse_string("l = \\ a b -> a", program.get());
    ASSERT_EQ(result, 0);


    ASSERT_EQ(program->bindings["l"]->get_form(), expform::abstraction);
    auto l = dynamic_cast<Abstraction*>(program->bindings["l"].get());
    auto args = l->args;
    EXPECT_EQ(args.size(), 2);
    EXPECT_EQ(std::count(args.begin(), args.end(), "a"), 1);
    EXPECT_EQ(std::count(args.begin(), args.end(), "b"), 1);
    ASSERT_EQ(l->body->get_form(), expform::variable);
    EXPECT_EQ(dynamic_cast<Variable*>(l->body.get())->name, "a");
}

#define TESTINFIXOP(opstr) {                                             \
    auto program = std::make_unique<Program>();                          \
    int result = parse_string("a = b " opstr " c", program.get());       \
    ASSERT_EQ(result, 0);                                                \
    ASSERT_EQ(program->bindings["a"]->get_form(), expform::application); \
    auto b = dynamic_cast<Application*>(program->bindings["a"].get());   \
    ASSERT_EQ(b->left->get_form(), expform::application);                \
    auto l = dynamic_cast<Application*>(b->left.get());                  \
    ASSERT_EQ(l->left->get_form(), expform::variable);                   \
    EXPECT_EQ(dynamic_cast<Variable*>(l->left.get())->name, (opstr));    \
    ASSERT_EQ(l->right->get_form(), expform::variable);                  \
    EXPECT_EQ(dynamic_cast<Variable*>(l->right.get())->name, "b");       \
    ASSERT_EQ(b->right->get_form(), expform::variable);                  \
    EXPECT_EQ(dynamic_cast<Variable*>(b->right.get())->name, "c");       \
}

TEST(Parser, ParsesInfix) {
    TESTINFIXOP("+");
    TESTINFIXOP("-");
    TESTINFIXOP("*");
    TESTINFIXOP("/");
    TESTINFIXOP("==");
    TESTINFIXOP("/=");
    TESTINFIXOP("<");
    TESTINFIXOP("<=");
    TESTINFIXOP(">");
    TESTINFIXOP(">=");
    TESTINFIXOP("&&");
    TESTINFIXOP("||");
    TESTINFIXOP(".");
    TESTINFIXOP("++");

    auto program = std::make_unique<Program>();
    auto result = parse_string("a = -c", program.get());
    ASSERT_EQ(result, 0);

    ASSERT_EQ(program->bindings["a"]->get_form(), expform::builtinop);
    auto a = dynamic_cast<BuiltInOp*>(program->bindings["a"].get());
    EXPECT_EQ(a->left, nullptr);
    ASSERT_EQ(a->right->get_form(), expform::variable);
    EXPECT_EQ(dynamic_cast<Variable*>(a->right.get())->name, "c");
    EXPECT_EQ(a->op, builtinop::negate);

    program = std::make_unique<Program>();
    result = parse_string("a = b:c", program.get());
    ASSERT_EQ(result, 0);

    ASSERT_EQ(program->bindings["a"]->get_form(), expform::application);
    auto b = dynamic_cast<Application*>(program->bindings["a"].get());
    ASSERT_EQ(b->left->get_form(), expform::application);
    auto l = dynamic_cast<Application*>(b->left.get());
    ASSERT_EQ(l->left->get_form(), expform::constructor);
    EXPECT_EQ(dynamic_cast<Constructor*>(l->left.get())->name, ":");
    ASSERT_EQ(l->right->get_form(), expform::variable);
    EXPECT_EQ(dynamic_cast<Variable*>(l->right.get())->name, "b");
    ASSERT_EQ(b->right->get_form(), expform::variable);
    EXPECT_EQ(dynamic_cast<Variable*>(b->right.get())->name, "c");
}

TEST(Parser, ParsesConditionals) {
    auto program = std::make_unique<Program>();
    auto result = parse_string("i = if a then 1 else 2", program.get());
    ASSERT_EQ(result, 0);


    ASSERT_EQ(program->bindings["i"]->get_form(), expform::cAsE);
    auto i = dynamic_cast<Case*>(program->bindings["i"].get());
    ASSERT_EQ(i->exp->get_form(), expform::variable);
    EXPECT_EQ(dynamic_cast<Variable*>(i->exp.get())->name, "a");

    const auto &alts = i->alts;
    ASSERT_EQ(alts.size(), 2);
    ASSERT_EQ(alts[0].first->get_form(), patternform::constructor);
    auto p = dynamic_cast<ConstructorPattern*>(alts[0].first.get());
    EXPECT_EQ(p->name, "True");
    EXPECT_EQ(p->args.size(), 0);
    ASSERT_EQ(alts[0].second->get_form(), expform::literal);
    auto l = dynamic_cast<Literal*>(alts[0].second.get());
    EXPECT_EQ(std::get<int>(l->value), 1);

    ASSERT_EQ(alts[1].first->get_form(), patternform::constructor);
    p = dynamic_cast<ConstructorPattern*>(alts[1].first.get());
    EXPECT_EQ(p->name, "False");
    EXPECT_EQ(p->args.size(), 0);
    ASSERT_EQ(alts[1].second->get_form(), expform::literal);
    l = dynamic_cast<Literal*>(alts[1].second.get());
    EXPECT_EQ(std::get<int>(l->value), 2);
}

TEST(Parser, ParsesLists) {
    auto program = std::make_unique<Program>();
    auto result = parse_string("l = [1,2]", program.get());
    ASSERT_EQ(result, 0);


    ASSERT_EQ(program->bindings["l"]->get_form(), expform::application);
    auto l = dynamic_cast<Application*>(program->bindings["l"].get());

    ASSERT_EQ(l->left->get_form(), expform::application);
    auto ll = dynamic_cast<Application*>(l->left.get());
    ASSERT_EQ(ll->left->get_form(), expform::constructor);
    EXPECT_EQ(dynamic_cast<Constructor*>(ll->left.get())->name, ":");
    ASSERT_EQ(ll->right->get_form(), expform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<Literal*>(ll->right.get())->value), 1);

    ASSERT_EQ(l->right->get_form(), expform::application);
    auto r = dynamic_cast<Application*>(l->right.get());
    ASSERT_EQ(r->left->get_form(), expform::application);
    auto rl = dynamic_cast<Application*>(r->left.get());
    ASSERT_EQ(rl->left->get_form(), expform::constructor);
    EXPECT_EQ(dynamic_cast<Constructor*>(rl->left.get())->name, ":");
    ASSERT_EQ(rl->right->get_form(), expform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<Literal*>(rl->right.get())->value), 2);
    ASSERT_EQ(r->right->get_form(), expform::constructor);
    EXPECT_EQ(dynamic_cast<Constructor*>(r->right.get())->name, "[]");
}

TEST(Parser, ParsesTuples) {
    auto program = std::make_unique<Program>();
    auto result = parse_string("l = (1,2)", program.get());
    ASSERT_EQ(result, 0);


    ASSERT_EQ(program->bindings["l"]->get_form(), expform::application);
    auto l = dynamic_cast<Application*>(program->bindings["l"].get());

    ASSERT_EQ(l->left->get_form(), expform::application);
    auto ll = dynamic_cast<Application*>(l->left.get());
    ASSERT_EQ(ll->left->get_form(), expform::constructor);
    EXPECT_EQ(dynamic_cast<Constructor*>(ll->left.get())->name, "(,)");
    ASSERT_EQ(ll->right->get_form(), expform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<Literal*>(ll->right.get())->value), 1);

    ASSERT_EQ(l->right->get_form(), expform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<Literal*>(l->right.get())->value), 2);
}

TEST(Parser, ParsesLetExpressions) {
    auto program = std::make_unique<Program>();
    EXPECT_THROW(parse_string("a = let {c=1;c=2} in c", program.get()), ParseError);

    program = std::make_unique<Program>();
    EXPECT_THROW(parse_string("a = let {c a = a; c=2} in c", program.get()), ParseError);

    program = std::make_unique<Program>();
    EXPECT_THROW(parse_string("a = let {c = a; c a =2} in c", program.get()), ParseError);

    program = std::make_unique<Program>();
    EXPECT_THROW(parse_string("a = let {c::Int; c::Double} in c", program.get()), ParseError);

    program = std::make_unique<Program>();
    auto result = parse_string("a = let {a :: b -> b; a x = x; p = 1} in p", program.get());
    ASSERT_EQ(result, 0);

    ASSERT_EQ(program->bindings["a"]->get_form(), expform::let);
    auto l = dynamic_cast<Let*>(program->bindings["a"].get());

    ASSERT_EQ(l->e->get_form(), expform::variable);
    EXPECT_EQ(dynamic_cast<Variable*>(l->e.get())->name, "p");

    EXPECT_EQ(l->bindings.size(), 2);

    EXPECT_EQ(l->bindings.at("a")->get_form(), expform::abstraction);
    auto lam = dynamic_cast<Abstraction*>(l->bindings.at("a").get());
    auto args = lam->args;
    EXPECT_EQ(args.size(), 1);
    EXPECT_EQ(std::count(args.begin(), args.end(), "x"), 1);
    ASSERT_EQ(lam->body->get_form(), expform::variable);
    EXPECT_EQ(dynamic_cast<Variable*>(lam->body.get())->name, "x");

    EXPECT_EQ(l->bindings.at("p")->get_form(), expform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<Literal*>(l->bindings.at("p").get())->value), 1);

    EXPECT_EQ(l->type_signatures.size(), 1);

    auto b = new UniversallyQuantifiedVariable("b");
    auto expected =make_function_type(b, b);
    EXPECT_TRUE(same_type(l->type_signatures.at("a").get(), expected));
}

TEST(Parser, ParsesCaseExpressions) {
    auto program = std::make_unique<Program>();
    auto result = parse_string("a = case 1 of {_ -> 2; _ -> 3}", program.get());
    ASSERT_EQ(result, 0);

    ASSERT_EQ(program->bindings["a"]->get_form(), expform::cAsE);
    auto c = dynamic_cast<Case*>(program->bindings["a"].get());

    EXPECT_EQ(c->exp->get_form(), expform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<Literal*>(c->exp.get())->value), 1);

    EXPECT_EQ(c->alts.size(), 2);
    EXPECT_EQ(c->alts[0].first->get_form(), patternform::wild);
    EXPECT_EQ(c->alts[1].first->get_form(), patternform::wild);

    EXPECT_EQ(c->alts[0].second->get_form(), expform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<Literal*>(c->alts[0].second.get())->value), 2);

    EXPECT_EQ(c->alts[1].second->get_form(), expform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<Literal*>(c->alts[1].second.get())->value), 3);
}

TEST(Parser, ParsesPatterns) {
    auto program = std::make_unique<Program>();
    auto result = parse_string("a = case 1 of {[1,2] -> 2}", program.get());
    ASSERT_EQ(result, 0);
    const auto &p = dynamic_cast<Case*>(program->bindings["a"].get())->alts[0].first;
    ASSERT_EQ(p->get_form(), patternform::constructor);
    auto c = dynamic_cast<ConstructorPattern*>(p.get());
    EXPECT_EQ(c->name, ":");
    EXPECT_EQ(c->args.size(), 2);
    ASSERT_EQ(c->args[0]->get_form(), patternform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<LiteralPattern*>(c->args[0].get())->value), 1);
    ASSERT_EQ(c->args[1]->get_form(), patternform::constructor);
    c = dynamic_cast<ConstructorPattern*>(c->args[1].get());
    EXPECT_EQ(c->name, ":");
    EXPECT_EQ(c->args.size(), 2);
    ASSERT_EQ(c->args[0]->get_form(), patternform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<LiteralPattern*>(c->args[0].get())->value), 2);
    ASSERT_EQ(c->args[1]->get_form(), patternform::constructor);
    c = dynamic_cast<ConstructorPattern*>(c->args[1].get());
    EXPECT_EQ(c->name, "[]");
    EXPECT_EQ(c->args.size(), 0);

    program = std::make_unique<Program>();
    result = parse_string("a = case 1 of {(1,2) -> 2}", program.get());
    ASSERT_EQ(result, 0);
    const auto &p1 = dynamic_cast<Case*>(program->bindings["a"].get())->alts[0].first;
    ASSERT_EQ(p1->get_form(), patternform::constructor);
    c = dynamic_cast<ConstructorPattern*>(p1.get());
    EXPECT_EQ(c->name, "(,)");
    EXPECT_EQ(c->args.size(), 2);
    ASSERT_EQ(c->args[0]->get_form(), patternform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<LiteralPattern*>(c->args[0].get())->value), 1);
    ASSERT_EQ(c->args[1]->get_form(), patternform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<LiteralPattern*>(c->args[1].get())->value), 2);

    program = std::make_unique<Program>();
    result = parse_string("a = case 1 of {Hi -> 2}", program.get());
    ASSERT_EQ(result, 0);
    const auto &p2 = dynamic_cast<Case*>(program->bindings["a"].get())->alts[0].first;
    ASSERT_EQ(p2->get_form(), patternform::constructor);
    c = dynamic_cast<ConstructorPattern*>(p2.get());
    EXPECT_EQ(c->name, "Hi");
    EXPECT_EQ(c->args.size(), 0);

    program = std::make_unique<Program>();
    result = parse_string("a = case 1 of {a@Hi -> 2}", program.get());
    ASSERT_EQ(result, 0);
    const auto &p3 = dynamic_cast<Case*>(program->bindings["a"].get())->alts[0].first;
    ASSERT_EQ(p3->get_form(), patternform::constructor);
    c = dynamic_cast<ConstructorPattern*>(p3.get());
    EXPECT_EQ(c->name, "Hi");
    EXPECT_EQ(c->args.size(), 0);
    ASSERT_EQ(c->as.size(), 1);
    EXPECT_EQ(c->as[0], "a");

    program = std::make_unique<Program>();
    result = parse_string("a = case 1 of {Hi a -> 2}", program.get());
    ASSERT_EQ(result, 0);
    const auto &p4 = dynamic_cast<Case*>(program->bindings["a"].get())->alts[0].first;
    ASSERT_EQ(p4->get_form(), patternform::constructor);
    c = dynamic_cast<ConstructorPattern*>(p4.get());
    EXPECT_EQ(c->name, "Hi");
    ASSERT_EQ(c->args.size(), 1);
    ASSERT_EQ(c->args[0]->get_form(), patternform::variable);
    auto v = dynamic_cast<VariablePattern*>(c->args[0].get());
    EXPECT_EQ(v->name, "a");

    program = std::make_unique<Program>();
    result = parse_string("a = case 1 of {(-2) -> 2}", program.get());
    ASSERT_EQ(result, 0);
    const auto &p5 = dynamic_cast<Case*>(program->bindings["a"].get())->alts[0].first;
    ASSERT_EQ(p5->get_form(), patternform::literal);
    EXPECT_EQ(std::get<int>(dynamic_cast<LiteralPattern*>(p5.get())->value), -2);

    program = std::make_unique<Program>();
    result = parse_string("a = case 1 of {a:b -> 2}", program.get());
    ASSERT_EQ(result, 0);
    const auto &p6 = dynamic_cast<Case*>(program->bindings["a"].get())->alts[0].first;
    ASSERT_EQ(p6->get_form(), patternform::constructor);
    c = dynamic_cast<ConstructorPattern*>(p6.get());
    EXPECT_EQ(c->name, ":");
    ASSERT_EQ(c->args.size(), 2);
    ASSERT_EQ(c->args[0]->get_form(), patternform::variable);
    EXPECT_EQ(dynamic_cast<VariablePattern*>(c->args[0].get())->name, "a");
    ASSERT_EQ(c->args[1]->get_form(), patternform::variable);
    EXPECT_EQ(dynamic_cast<VariablePattern*>(c->args[1].get())->name, "b");
}
