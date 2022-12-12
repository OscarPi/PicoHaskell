#include <gtest/gtest.h>
#include "types/types.hpp"

TEST(Types, KindEquality) {
    ASSERT_TRUE(sameKind(kStar, kStar));
    ASSERT_TRUE(sameKind(kStar, std::make_shared<const StarKind>()));
    ASSERT_TRUE(sameKind(kStarToStar, kStarToStar));
    ASSERT_TRUE(sameKind(kStarToStarToStar, kStarToStarToStar));

    ASSERT_FALSE(sameKind(kStar, kStarToStar));
    ASSERT_FALSE(sameKind(kStar, kStarToStarToStar));
    ASSERT_FALSE(sameKind(kStarToStar, kStar));
    ASSERT_FALSE(sameKind(kStarToStar, kStarToStarToStar));
    ASSERT_FALSE(sameKind(kStarToStarToStar, kStar));
    ASSERT_FALSE(sameKind(kStarToStarToStar, kStarToStar));
}

TEST(Types, TypeEquality) {
    ASSERT_TRUE(sameType(tUnit, tUnit));
    ASSERT_TRUE(sameType(tUnit, std::make_shared<const TypeConstructor>("()", kStar)));
    ASSERT_TRUE(sameType(tChar, tChar));
    ASSERT_TRUE(sameType(tList, tList));
    ASSERT_TRUE(sameType(tArrow, tArrow));
    ASSERT_TRUE(sameType(tTuple2, tTuple2));

    ASSERT_FALSE(sameType(tUnit, tChar));
    ASSERT_FALSE(sameType(tUnit, tList));
    ASSERT_FALSE(sameType(tUnit, tArrow));
    ASSERT_FALSE(sameType(tUnit, tTuple2));
    ASSERT_FALSE(sameType(tList, tUnit));
    ASSERT_FALSE(sameType(tList, tArrow));
    ASSERT_FALSE(sameType(tList, tTuple2));
    ASSERT_FALSE(sameType(tArrow, tUnit));
    ASSERT_FALSE(sameType(tArrow, tList));
    ASSERT_FALSE(sameType(tArrow, tTuple2));
    ASSERT_FALSE(sameType(tTuple2, tUnit));
    ASSERT_FALSE(sameType(tTuple2, tList));
    ASSERT_FALSE(sameType(tTuple2, tArrow));
}

TEST(Types, MakeFunctionType) {
    type partial = std::make_shared<const TypeApplication>(tArrow, tDouble);
    ASSERT_TRUE(sameType(makeFunctionType(tDouble, tInt), std::make_shared<const TypeApplication>(partial, tInt)));
}

TEST(Types, MakeListType) {
    ASSERT_TRUE(sameType(makeListType(tInt), std::make_shared<const TypeApplication>(tList, tInt)));
}

TEST(Types, MakePairType) {
    type partial = std::make_shared<const TypeApplication>(tTuple2, tInt);
    ASSERT_TRUE(sameType(makePairType(tInt, tDouble), std::make_shared<const TypeApplication>(partial, tDouble)));
}

TEST(Types, ApplySubstitution) {
    substitution s;
    type varA = std::make_shared<TypeVariable>("a", kStar);
    type varB = std::make_shared<TypeVariable>("b", kStarToStar);
    type con1 = std::make_shared<TypeConstructor>("a", kStar);
    type con2 = std::make_shared<TypeConstructor>("b", kStar);
    type con3 = std::make_shared<TypeConstructor>("c", kStarToStarToStar);
    type ap1 = std::make_shared<TypeApplication>(con3, varA);
    type ap2 = std::make_shared<TypeApplication>(varB, varA);
    type ap3 = std::make_shared<TypeApplication>(ap1, con1);

    ASSERT_TRUE(sameType(varA, applySubstitution(varA, s)));
    ASSERT_TRUE(sameType(varB, applySubstitution(varB, s)));
    ASSERT_TRUE(sameType(con1, applySubstitution(con1, s)));
    ASSERT_TRUE(sameType(con2, applySubstitution(con2, s)));
    ASSERT_TRUE(sameType(con3, applySubstitution(con3, s)));
    ASSERT_TRUE(sameType(ap1, applySubstitution(ap1, s)));
    ASSERT_TRUE(sameType(ap2, applySubstitution(ap2, s)));
    ASSERT_TRUE(sameType(ap3, applySubstitution(ap3, s)));

    s["a"] = tUnit;
    type ap1Dash = std::make_shared<TypeApplication>(con3, tUnit);
    ASSERT_TRUE(sameType(tUnit, applySubstitution(varA, s)));
    ASSERT_TRUE(sameType(varB, applySubstitution(varB, s)));
    ASSERT_TRUE(sameType(con1, applySubstitution(con1, s)));
    ASSERT_TRUE(sameType(con2, applySubstitution(con2, s)));
    ASSERT_TRUE(sameType(con3, applySubstitution(con3, s)));
    ASSERT_TRUE(sameType(ap1Dash, applySubstitution(ap1, s)));
    ASSERT_TRUE(sameType(std::make_shared<TypeApplication>(varB, tUnit), applySubstitution(ap2, s)));
    ASSERT_TRUE(sameType(std::make_shared<TypeApplication>(ap1Dash, con1), applySubstitution(ap3, s)));

    s["b"] = tInt;
    ASSERT_TRUE(sameType(tUnit, applySubstitution(varA, s)));
    ASSERT_TRUE(sameType(tInt, applySubstitution(varB, s)));
    ASSERT_TRUE(sameType(con1, applySubstitution(con1, s)));
    ASSERT_TRUE(sameType(con2, applySubstitution(con2, s)));
    ASSERT_TRUE(sameType(con3, applySubstitution(con3, s)));
    ASSERT_TRUE(sameType(ap1Dash, applySubstitution(ap1, s)));
    ASSERT_TRUE(sameType(std::make_shared<TypeApplication>(tInt, tUnit), applySubstitution(ap2, s)));
    ASSERT_TRUE(sameType(std::make_shared<TypeApplication>(ap1Dash, con1), applySubstitution(ap3, s)));

    s.erase("a");
    ASSERT_TRUE(sameType(varA, applySubstitution(varA, s)));
    ASSERT_TRUE(sameType(tInt, applySubstitution(varB, s)));
    ASSERT_TRUE(sameType(con1, applySubstitution(con1, s)));
    ASSERT_TRUE(sameType(con2, applySubstitution(con2, s)));
    ASSERT_TRUE(sameType(con3, applySubstitution(con3, s)));
    ASSERT_TRUE(sameType(ap1, applySubstitution(ap1, s)));
    ASSERT_TRUE(sameType(std::make_shared<TypeApplication>(tInt, varA), applySubstitution(ap2, s)));
    ASSERT_TRUE(sameType(std::make_shared<TypeApplication>(ap1, con1), applySubstitution(ap3, s)));
}

TEST(Types, FindTypeVariables) {
    type varA = std::make_shared<TypeVariable>("a", kStar);
    auto variables = findTypeVariables(varA);
    ASSERT_EQ(variables.size(), 1);
    ASSERT_TRUE(variables.count("a") == 1);

    type varB = std::make_shared<TypeVariable>("b", kStarToStar);
    variables = findTypeVariables(varB);
    ASSERT_EQ(variables.size(), 1);
    ASSERT_TRUE(variables.count("b") == 1);

    type con1 = std::make_shared<TypeConstructor>("a", kStar);
    variables = findTypeVariables(con1);
    ASSERT_EQ(variables.size(), 0);

    type ap1 = std::make_shared<TypeApplication>(varB, varA);
    variables = findTypeVariables(ap1);
    ASSERT_EQ(variables.size(), 2);
    ASSERT_TRUE(variables.count("a") == 1);
    ASSERT_TRUE(variables.count("b") == 1);

    type ap3 = std::make_shared<TypeApplication>(ap1, con1);
    variables = findTypeVariables(ap1);
    ASSERT_EQ(variables.size(), 2);
    ASSERT_TRUE(variables.count("a") == 1);
    ASSERT_TRUE(variables.count("b") == 1);
}

TEST(Types, ApplySubstitutionVector) {
    substitution s;
    s["a"] = tUnit;
    std::vector<type> ts;
    ts.push_back(std::make_shared<TypeVariable>("a", kStar));
    ts.push_back(std::make_shared<TypeVariable>("b", kStarToStar));
    ts.push_back(std::make_shared<TypeConstructor>("a", kStar));
    ts.push_back(std::make_shared<TypeConstructor>("b", kStar));
    ts.push_back(std::make_shared<TypeConstructor>("c", kStarToStarToStar));
    ts.push_back(std::make_shared<TypeApplication>(ts[4], ts[0]));
    ts.push_back(std::make_shared<TypeApplication>(ts[1], ts[0]));
    ts.push_back(std::make_shared<TypeApplication>(ts[5], ts[2]));

    std::vector<type> substituted = applySubstitution(ts, s);
    ASSERT_EQ(substituted.size(), 8);
    type ap1Dash = std::make_shared<TypeApplication>(ts[4], tUnit);
    ASSERT_TRUE(sameType(tUnit, substituted[0]));
    ASSERT_TRUE(sameType(ts[1], substituted[1]));
    ASSERT_TRUE(sameType(ts[2], substituted[2]));
    ASSERT_TRUE(sameType(ts[3], substituted[3]));
    ASSERT_TRUE(sameType(ts[4], substituted[4]));
    ASSERT_TRUE(sameType(ap1Dash, substituted[5]));
    ASSERT_TRUE(sameType(std::make_shared<TypeApplication>(ts[1], tUnit), substituted[6]));
    ASSERT_TRUE(sameType(std::make_shared<TypeApplication>(ap1Dash, ts[2]), substituted[7]));
}

TEST(Types, FindTypeVariablesVector) {
    std::vector<type> ts;
    ts.push_back(std::make_shared<TypeVariable>("a", kStar));
    ts.push_back(std::make_shared<TypeVariable>("b", kStarToStar));
    ts.push_back(std::make_shared<TypeConstructor>("c", kStar));
    ts.push_back(std::make_shared<TypeApplication>(std::make_shared<TypeVariable>("d", kStarToStar), tUnit));
    auto variables = findTypeVariables(ts);
    ASSERT_EQ(variables.size(), 3);
    ASSERT_TRUE(variables.count("a") == 1);
    ASSERT_TRUE(variables.count("b") == 1);
    ASSERT_TRUE(variables.count("d") == 1);
}
