#include <gtest/gtest.h>
#include <algorithm>
#include "test/test_utilities.hpp"
#include "types/types.hpp"
#include "parser/syntax.hpp"
#include "types/type_check.hpp"

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

#define EXPECT_WELL_TYPED(str) {                                    \
    std::unique_ptr<Program> program = std::make_unique<Program>(); \
    int result = parse_string((str), program.get());                \
    ASSERT_EQ(result, 0);                                           \
    type_check(program, false);                                     \
}

#define EXPECT_NOT_WELL_TYPED(str) {                                \
    std::unique_ptr<Program> program = std::make_unique<Program>(); \
    int result = parse_string((str), program.get());                \
    ASSERT_EQ(result, 0);                                           \
    EXPECT_THROW(type_check(program, false), TypeError);            \
}

TEST(Types, Literals) {
    EXPECT_WELL_TYPED("b = 1");
    EXPECT_WELL_TYPED("b :: Int\n;b = 1");
    EXPECT_NOT_WELL_TYPED("b :: Char\n;b = 1");

    EXPECT_WELL_TYPED("b = 'a'");
    EXPECT_WELL_TYPED("b :: Char\n;b = 'a'");
    EXPECT_NOT_WELL_TYPED("b :: Int\n;b = 'a'");

    EXPECT_WELL_TYPED("b = \"ha\"");
    EXPECT_NOT_WELL_TYPED("b :: Char\n;b = \"ha\"");
    EXPECT_WELL_TYPED("b :: [Char]\n;b = \"ha\"");
    EXPECT_WELL_TYPED("b :: [] Char\n;b = \"ha\"");

    EXPECT_WELL_TYPED("b = True");
    EXPECT_WELL_TYPED("b :: Bool\n;b = True");
    EXPECT_NOT_WELL_TYPED("b :: Char\n;b = True");

    EXPECT_WELL_TYPED("b = False");
    EXPECT_WELL_TYPED("b :: Bool\n;b = False");
    EXPECT_NOT_WELL_TYPED("b :: Char\n;b = False");
}

TEST(Types, BuiltInOperators) {
    EXPECT_WELL_TYPED("a :: Int\n;a = 1 + 2");
    EXPECT_NOT_WELL_TYPED("a = 1 + False");
    EXPECT_NOT_WELL_TYPED("a = 'b' + 3");
    EXPECT_NOT_WELL_TYPED("a = 'b' + 'a'");

    EXPECT_WELL_TYPED("a :: Int\n;a = 1 - 2");
    EXPECT_NOT_WELL_TYPED("a = 1 - False");
    EXPECT_NOT_WELL_TYPED("a = 'b' - 3");
    EXPECT_NOT_WELL_TYPED("a = 'b' - 'a'");

    EXPECT_WELL_TYPED("a :: Int\n;a = 1 * 2");
    EXPECT_NOT_WELL_TYPED("a = 1 * False");
    EXPECT_NOT_WELL_TYPED("a = 'b' * 3");
    EXPECT_NOT_WELL_TYPED("a = 'b' * 'a'");

    EXPECT_WELL_TYPED("a :: Int\n;a = 1 / 2");
    EXPECT_NOT_WELL_TYPED("a = 1 / False");
    EXPECT_NOT_WELL_TYPED("a = 'b' / 3");
    EXPECT_NOT_WELL_TYPED("a = 'b' / 'a'");

    EXPECT_WELL_TYPED("a :: Int\n;a = -x\n;x = 1");
    EXPECT_NOT_WELL_TYPED("a = -x\n;x = 'a'");

    EXPECT_WELL_TYPED("a :: Bool\n;a = 1==2");
    EXPECT_WELL_TYPED("a :: Bool\n;a = True==False");
    EXPECT_WELL_TYPED("a :: Bool\n;a = 'a'=='b'");
    EXPECT_NOT_WELL_TYPED("a :: Bool\n;a = True=='b'");

    EXPECT_WELL_TYPED("a :: Bool\n;a = 1/=2");
    EXPECT_WELL_TYPED("a :: Bool\n;a = True/=False");
    EXPECT_WELL_TYPED("a :: Bool\n;a = 'a'/='b'");
    EXPECT_NOT_WELL_TYPED("a :: Bool\n;a = True/='b'");

    EXPECT_WELL_TYPED("a :: Bool\n;a = 1 < 2");
    EXPECT_NOT_WELL_TYPED("a = 1 < False");
    EXPECT_NOT_WELL_TYPED("a = 'b' < 3");
    EXPECT_NOT_WELL_TYPED("a = 'b' < 'a'");

    EXPECT_WELL_TYPED("a :: Bool\n;a = 1 <= 2");
    EXPECT_NOT_WELL_TYPED("a = 1 <= False");
    EXPECT_NOT_WELL_TYPED("a = 'b' <= 3");
    EXPECT_NOT_WELL_TYPED("a = 'b' <= 'a'");

    EXPECT_WELL_TYPED("a :: Bool\n;a = 1 > 2");
    EXPECT_NOT_WELL_TYPED("a = 1 > False");
    EXPECT_NOT_WELL_TYPED("a = 'b' > 3");
    EXPECT_NOT_WELL_TYPED("a = 'b' > 'a'");

    EXPECT_WELL_TYPED("a :: Bool\n;a = 1 >= 2");
    EXPECT_NOT_WELL_TYPED("a = 1 >= False");
    EXPECT_NOT_WELL_TYPED("a = 'b' >= 3");
    EXPECT_NOT_WELL_TYPED("a = 'b' >= 'a'");

    EXPECT_WELL_TYPED("a :: Bool\n;a = True && False");
    EXPECT_NOT_WELL_TYPED("a = 1 && False");
    EXPECT_NOT_WELL_TYPED("a = 'b' && 3");
    EXPECT_NOT_WELL_TYPED("a = 1 && 2");

    EXPECT_WELL_TYPED("a :: Bool\n;a = True || False");
    EXPECT_NOT_WELL_TYPED("a = 1 || False");
    EXPECT_NOT_WELL_TYPED("a = 'b' || 3");
    EXPECT_NOT_WELL_TYPED("a = 1 || 2");
}

TEST(Types, Functions) {
    EXPECT_WELL_TYPED(
            "a :: Int -> Int -> Int -> Int;"
            "a = \\x y z -> x + y + z");
    EXPECT_WELL_TYPED(
            "b :: Int -> Int -> Int;"
            "b = a 1;"
            "a :: Int -> Int -> Int -> Int;"
            "a = \\x y z -> x + y + z;"
            "c :: Int;"
            "c = b 2 3");
    EXPECT_NOT_WELL_TYPED(
            "a :: Int -> Int -> Int -> Int;"
            "a = \\x y z -> x + y + z;"
            "c = a 'a'");
    EXPECT_NOT_WELL_TYPED(
            "a :: Int -> Int -> Int -> Int;"
            "a = \\x y z -> x + y + z;"
            "c = a 1 2 3 4");
}

TEST(Types, UndefinedNames) {
    EXPECT_NOT_WELL_TYPED("a x = b");
    EXPECT_NOT_WELL_TYPED("a x = B x");
    EXPECT_WELL_TYPED("a x = b\n;b = 1");
    EXPECT_WELL_TYPED("data H = B Int\n;a x = B x");
}

TEST(Types, Conditionals) {
    EXPECT_NOT_WELL_TYPED(
            "a = if 'a' then 3 else 4");
    EXPECT_NOT_WELL_TYPED(
            "a = if True then 'a' else 4");
    EXPECT_WELL_TYPED(
            "a :: Int;"
            "a = if True then 3 else 4");
    EXPECT_WELL_TYPED(
            "a :: [Char];"
            "a = if False then \"yay\" else \"oops\"");
}

TEST(Types, Lists) {
    EXPECT_WELL_TYPED(
            "a :: [b];"
            "a = []");
    EXPECT_NOT_WELL_TYPED(
            "a :: [b];"
            "a = [1]");
    EXPECT_WELL_TYPED(
            "a :: [Char];"
            "a = 'a':'b':'c':[]");
    EXPECT_NOT_WELL_TYPED(
            "a = [1, True]");
}

TEST(Types, Tuples) {
    EXPECT_WELL_TYPED(
            "x :: (Int, Char, Bool);"
            "x = (1, 'a', True)");
    EXPECT_NOT_WELL_TYPED(
            "x :: (Bool, Char, Int);"
            "x = (1, 'a', True)");
    EXPECT_WELL_TYPED(
            "x :: ();"
            "x = ()");
    EXPECT_NOT_WELL_TYPED(
            "x :: (Int, Int, Int, Int, Int);"
            "x = (1, 2)");
}

TEST(Types, Let) {
    EXPECT_WELL_TYPED(
            "x :: Int;"
            "x = 10;"
            "b :: Char;"
            "b = let {x :: Char; x = 'a'} in x");
    EXPECT_NOT_WELL_TYPED(
            "a = let {c::Int} in 1");
    EXPECT_NOT_WELL_TYPED(
            "x = 'a';"
            "b :: Char;"
            "b = let {x :: Char} in x");
    EXPECT_WELL_TYPED(
            "a :: Int;"
            "a = let {c x = x + 1; b = 1} in c b");
    EXPECT_WELL_TYPED(
            "f :: Bool -> (a -> ([Bool], a), b -> ([Bool], b));"
            "f x = let { g y z = ([x,y], z) } in (g True, g False)");
    EXPECT_NOT_WELL_TYPED(
            "f x = let { g y z = ([x,y], z) } in (g True, g 'c')");
    EXPECT_NOT_WELL_TYPED(
            "f x = let { g :: a -> b -> ([a], b); g y z = ([x,y], z) } in g");
    EXPECT_WELL_TYPED(
            "f x = let { g :: Int -> b -> ([Int], b); g y z = ([x,y], z) } in g");
}

TEST(Types, Case) {
    EXPECT_NOT_WELL_TYPED(
            "data Tree a = Leaf | Node a (Tree a) (Tree a);"
            "k = case Leaf of {Node 1 -> \"Hi\"}");
    EXPECT_NOT_WELL_TYPED(
            "data Tree a = Leaf | Node a (Tree a) (Tree a);"
            "k = case Leaf of {Node 1 (Node 'a' Leaf Leaf) (Node 'b' Leaf Leaf) -> \"Hi\"}");
    EXPECT_WELL_TYPED(
            "data Tree a = Leaf | Node a (Tree a) (Tree a);"
            "k = case Leaf of {Node 1 (Node 1 Leaf Leaf) (Node 2 Leaf Leaf) -> \"Hi\"}");
    EXPECT_WELL_TYPED(
            "k = case [] of {x:xs -> 1; [] -> 2}");
    EXPECT_NOT_WELL_TYPED(
            "k = case (1,2) of {(a,b,c) -> 1; (a,b) -> 2}");
    EXPECT_WELL_TYPED(
            "k = case (1,2) of {(a,b) -> 1; (a,b) -> 2}");
    EXPECT_NOT_WELL_TYPED(
            "k = case (1,2) of {(a,a) -> 1}");
    EXPECT_WELL_TYPED(
            "k = case 3 of { -5 -> 1 }");
    EXPECT_WELL_TYPED(
            "k :: Int;"
            "k = case 3 of { i@(-5) -> i }");
    EXPECT_WELL_TYPED(
            "k :: Int;"
            "k = case (1,2,3) of { (_,_,z) -> z }");
}

TEST(Types, KindInference) {
    EXPECT_WELL_TYPED(
            "data App f a = A (f a);"
            "data Tree a = Leaf | Fork (Tree a) (Tree a)");
    EXPECT_NOT_WELL_TYPED(
            "data App f a = A (f a);"
            "data Tree a = Leaf | Fork (Tree a) (Tree a);"
            "data Funny = F (Tree [])");
    EXPECT_WELL_TYPED(
            "data A = B C;"
            "data C = D A");
}

TEST(Types, CheckForMain) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    int result = parse_string("main = \"abc\"", program.get());
    ASSERT_EQ(result, 0);
    type_check(program, true);

    program = std::make_unique<Program>();
    result = parse_string("main = 'a'", program.get());
    ASSERT_EQ(result, 0);
    EXPECT_THROW(type_check(program, true), TypeError);

    program = std::make_unique<Program>();
    result = parse_string("a = \"a\"", program.get());
    ASSERT_EQ(result, 0);
    EXPECT_THROW(type_check(program, true), TypeError);
}
