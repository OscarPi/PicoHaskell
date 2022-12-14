#include <gtest/gtest.h>
#include <algorithm>
#include "types/types.hpp"

TEST(Types, KindEquality) {
    EXPECT_TRUE(sameKind(kStar, kStar));
    EXPECT_TRUE(sameKind(kStar, std::make_shared<const StarKind>()));
    EXPECT_TRUE(sameKind(kStarToStar, kStarToStar));
    EXPECT_TRUE(sameKind(kStarToStarToStar, kStarToStarToStar));

    EXPECT_FALSE(sameKind(kStar, kStarToStar));
    EXPECT_FALSE(sameKind(kStar, kStarToStarToStar));
    EXPECT_FALSE(sameKind(kStarToStar, kStar));
    EXPECT_FALSE(sameKind(kStarToStar, kStarToStarToStar));
    EXPECT_FALSE(sameKind(kStarToStarToStar, kStar));
    EXPECT_FALSE(sameKind(kStarToStarToStar, kStarToStar));
}

TEST(Types, TypeEquality) {
    EXPECT_TRUE(sameType(tUnit, tUnit));
    EXPECT_TRUE(sameType(tUnit, std::make_shared<const TypeConstructor>("()", kStar)));
    EXPECT_TRUE(sameType(tChar, tChar));
    EXPECT_TRUE(sameType(tList, tList));
    EXPECT_TRUE(sameType(tArrow, tArrow));
    EXPECT_TRUE(sameType(tTuple2, tTuple2));

    EXPECT_FALSE(sameType(tUnit, tChar));
    EXPECT_FALSE(sameType(tUnit, tList));
    EXPECT_FALSE(sameType(tUnit, tArrow));
    EXPECT_FALSE(sameType(tUnit, tTuple2));
    EXPECT_FALSE(sameType(tList, tUnit));
    EXPECT_FALSE(sameType(tList, tArrow));
    EXPECT_FALSE(sameType(tList, tTuple2));
    EXPECT_FALSE(sameType(tArrow, tUnit));
    EXPECT_FALSE(sameType(tArrow, tList));
    EXPECT_FALSE(sameType(tArrow, tTuple2));
    EXPECT_FALSE(sameType(tTuple2, tUnit));
    EXPECT_FALSE(sameType(tTuple2, tList));
    EXPECT_FALSE(sameType(tTuple2, tArrow));
}

TEST(Types, MakeFunctionType) {
    type partial = std::make_shared<const TypeApplication>(tArrow, tDouble);
    EXPECT_TRUE(sameType(makeFunctionType(tDouble, tInt), std::make_shared<const TypeApplication>(partial, tInt)));
}

TEST(Types, MakeListType) {
    EXPECT_TRUE(sameType(makeListType(tInt), std::make_shared<const TypeApplication>(tList, tInt)));
}

TEST(Types, MakePairType) {
    type partial = std::make_shared<const TypeApplication>(tTuple2, tInt);
    EXPECT_TRUE(sameType(makePairType(tInt, tDouble), std::make_shared<const TypeApplication>(partial, tDouble)));
}

TEST(Types, ApplySubstitution) {
    substitution s;
    type varA = std::make_shared<const TypeVariable>("a", kStar);
    type varB = std::make_shared<const TypeVariable>("b", kStarToStar);
    type con1 = std::make_shared<const TypeConstructor>("a", kStar);
    type con2 = std::make_shared<const TypeConstructor>("b", kStar);
    type con3 = std::make_shared<const TypeConstructor>("c", kStarToStarToStar);
    type ap1 = std::make_shared<const TypeApplication>(con3, varA);
    type ap2 = std::make_shared<const TypeApplication>(varB, varA);
    type ap3 = std::make_shared<const TypeApplication>(ap1, con1);

    EXPECT_TRUE(sameType(varA, applySubstitution(varA, s)));
    EXPECT_TRUE(sameType(varB, applySubstitution(varB, s)));
    EXPECT_TRUE(sameType(con1, applySubstitution(con1, s)));
    EXPECT_TRUE(sameType(con2, applySubstitution(con2, s)));
    EXPECT_TRUE(sameType(con3, applySubstitution(con3, s)));
    EXPECT_TRUE(sameType(ap1, applySubstitution(ap1, s)));
    EXPECT_TRUE(sameType(ap2, applySubstitution(ap2, s)));
    EXPECT_TRUE(sameType(ap3, applySubstitution(ap3, s)));

    s["a"] = tUnit;
    type ap1Dash = std::make_shared<const TypeApplication>(con3, tUnit);
    EXPECT_TRUE(sameType(tUnit, applySubstitution(varA, s)));
    EXPECT_TRUE(sameType(varB, applySubstitution(varB, s)));
    EXPECT_TRUE(sameType(con1, applySubstitution(con1, s)));
    EXPECT_TRUE(sameType(con2, applySubstitution(con2, s)));
    EXPECT_TRUE(sameType(con3, applySubstitution(con3, s)));
    EXPECT_TRUE(sameType(ap1Dash, applySubstitution(ap1, s)));
    EXPECT_TRUE(sameType(std::make_shared<const TypeApplication>(varB, tUnit), applySubstitution(ap2, s)));
    EXPECT_TRUE(sameType(std::make_shared<const TypeApplication>(ap1Dash, con1), applySubstitution(ap3, s)));

    s["b"] = tInt;
    EXPECT_TRUE(sameType(tUnit, applySubstitution(varA, s)));
    EXPECT_TRUE(sameType(tInt, applySubstitution(varB, s)));
    EXPECT_TRUE(sameType(con1, applySubstitution(con1, s)));
    EXPECT_TRUE(sameType(con2, applySubstitution(con2, s)));
    EXPECT_TRUE(sameType(con3, applySubstitution(con3, s)));
    EXPECT_TRUE(sameType(ap1Dash, applySubstitution(ap1, s)));
    EXPECT_TRUE(sameType(std::make_shared<const TypeApplication>(tInt, tUnit), applySubstitution(ap2, s)));
    EXPECT_TRUE(sameType(std::make_shared<const TypeApplication>(ap1Dash, con1), applySubstitution(ap3, s)));

    s.erase("a");
    EXPECT_TRUE(sameType(varA, applySubstitution(varA, s)));
    EXPECT_TRUE(sameType(tInt, applySubstitution(varB, s)));
    EXPECT_TRUE(sameType(con1, applySubstitution(con1, s)));
    EXPECT_TRUE(sameType(con2, applySubstitution(con2, s)));
    EXPECT_TRUE(sameType(con3, applySubstitution(con3, s)));
    EXPECT_TRUE(sameType(ap1, applySubstitution(ap1, s)));
    EXPECT_TRUE(sameType(std::make_shared<const TypeApplication>(tInt, varA), applySubstitution(ap2, s)));
    EXPECT_TRUE(sameType(std::make_shared<const TypeApplication>(ap1, con1), applySubstitution(ap3, s)));
}

TEST(Types, FindTypeVariables) {
    type varA = std::make_shared<const TypeVariable>("a", kStar);
    auto variables = findTypeVariables(varA);
    EXPECT_EQ(variables.size(), 1);
    EXPECT_TRUE(std::count(variables.begin(), variables.end(), "a") == 1);

    type varB = std::make_shared<const TypeVariable>("b", kStarToStar);
    variables = findTypeVariables(varB);
    EXPECT_EQ(variables.size(), 1);
    EXPECT_TRUE(std::count(variables.begin(), variables.end(), "b") == 1);

    type con1 = std::make_shared<const TypeConstructor>("a", kStar);
    variables = findTypeVariables(con1);
    EXPECT_EQ(variables.size(), 0);

    type ap1 = std::make_shared<const TypeApplication>(varB, varA);
    variables = findTypeVariables(ap1);
    EXPECT_EQ(variables.size(), 2);
    EXPECT_TRUE(std::count(variables.begin(), variables.end(), "a") == 1);
    EXPECT_TRUE(std::count(variables.begin(), variables.end(), "b") == 1);

    type ap3 = std::make_shared<const TypeApplication>(ap1, con1);
    variables = findTypeVariables(ap1);
    EXPECT_EQ(variables.size(), 2);
    EXPECT_TRUE(std::count(variables.begin(), variables.end(), "a") == 1);
    EXPECT_TRUE(std::count(variables.begin(), variables.end(), "b") == 1);
}

TEST(Types, ApplySubstitutionVector) {
    substitution s;
    s["a"] = tUnit;
    std::vector<type> ts;
    ts.push_back(std::make_shared<const TypeVariable>("a", kStar));
    ts.push_back(std::make_shared<const TypeVariable>("b", kStarToStar));
    ts.push_back(std::make_shared<const TypeConstructor>("a", kStar));
    ts.push_back(std::make_shared<const TypeConstructor>("b", kStar));
    ts.push_back(std::make_shared<const TypeConstructor>("c", kStarToStarToStar));
    ts.push_back(std::make_shared<const TypeApplication>(ts[4], ts[0]));
    ts.push_back(std::make_shared<const TypeApplication>(ts[1], ts[0]));
    ts.push_back(std::make_shared<const TypeApplication>(ts[5], ts[2]));

    std::vector<type> substituted = applySubstitution(ts, s);
    EXPECT_EQ(substituted.size(), 8);
    type ap1Dash = std::make_shared<const TypeApplication>(ts[4], tUnit);
    EXPECT_TRUE(sameType(tUnit, substituted[0]));
    EXPECT_TRUE(sameType(ts[1], substituted[1]));
    EXPECT_TRUE(sameType(ts[2], substituted[2]));
    EXPECT_TRUE(sameType(ts[3], substituted[3]));
    EXPECT_TRUE(sameType(ts[4], substituted[4]));
    EXPECT_TRUE(sameType(ap1Dash, substituted[5]));
    EXPECT_TRUE(sameType(std::make_shared<const TypeApplication>(ts[1], tUnit), substituted[6]));
    EXPECT_TRUE(sameType(std::make_shared<const TypeApplication>(ap1Dash, ts[2]), substituted[7]));
}

TEST(Types, FindTypeVariablesVector) {
    std::vector<type> ts;
    ts.push_back(std::make_shared<const TypeVariable>("a", kStar));
    ts.push_back(std::make_shared<const TypeVariable>("b", kStarToStar));
    ts.push_back(std::make_shared<const TypeConstructor>("c", kStar));
    ts.push_back(std::make_shared<const TypeApplication>(std::make_shared<const TypeVariable>("d", kStarToStar), tUnit));
    auto variables = findTypeVariables(ts);
    EXPECT_EQ(variables.size(), 3);
    EXPECT_TRUE(std::count(variables.begin(), variables.end(), "a") == 1);
    EXPECT_TRUE(std::count(variables.begin(), variables.end(), "b") == 1);
    EXPECT_TRUE(std::count(variables.begin(), variables.end(), "d") == 1);
}

TEST(Types, SubstitutionComposition) {
    substitution s1;
    substitution s2;
    s2["a"] = tUnit;
    s1["a"] = tInt;
    s2["b"] = std::make_shared<const TypeVariable>("c", kStar);
    s1["c"] = tDouble;
    s1["d"] = std::make_shared<const TypeVariable>("e", kStar);
    s2["e"] = tFloat;

    substitution s = compose(s1, s2);

    EXPECT_EQ(s.size(), 5);
    EXPECT_TRUE(sameType(s["a"], tUnit));
    EXPECT_TRUE(sameType(s["b"], tDouble));
    EXPECT_TRUE(sameType(s["c"], tDouble));
    EXPECT_TRUE(sameType(s["d"], std::make_shared<const TypeVariable>("e", kStar)));
    EXPECT_TRUE(sameType(s["e"], tFloat));
}

TEST(Types, SubstitutionMerge) {
    substitution s1;
    substitution s2;
    s1["a"] = tUnit;
    s2["b"] = tDouble;

    substitution s = merge(s1, s2);
    EXPECT_EQ(s.size(), 2);
    EXPECT_TRUE(sameType(s["a"], tUnit));
    EXPECT_TRUE(sameType(s["b"], tDouble));

    s2["a"] = tUnit;
    s1["b"] = tDouble;
    s = merge(s1, s2);
    EXPECT_EQ(s.size(), 2);
    EXPECT_TRUE(sameType(s["a"], tUnit));
    EXPECT_TRUE(sameType(s["b"], tDouble));

    s1["a"] = tDouble;
    EXPECT_THROW(merge(s1, s2), std::invalid_argument);

    s1["a"] = std::make_shared<const TypeVariable>("b", kStar);
    s2["b"] = std::make_shared<const TypeVariable>("a", kStar);
    s1.erase("b");
    s2.erase("a");
    s = merge(s1, s2);
    EXPECT_EQ(s.size(), 2);
    EXPECT_TRUE(sameType(s["a"], std::make_shared<const TypeVariable>("b", kStar)));
    EXPECT_TRUE(sameType(s["b"], std::make_shared<const TypeVariable>("a", kStar)));
}

TEST(Types, Unification) {
    type t1 = std::make_shared<const TypeVariable>("a", kStar);
    type t2 = std::make_shared<const TypeVariable>("a", kStar);
    substitution s = mostGeneralUnifier(t1, t2);
    EXPECT_EQ(s.size(), 0);

    t2 = std::make_shared<const TypeVariable>("a", kStarToStar);
    EXPECT_THROW(mostGeneralUnifier(t1, t2), std::invalid_argument);

    t2 = std::make_shared<const TypeApplication>(tList, t1);
    EXPECT_THROW(mostGeneralUnifier(t1, t2), std::invalid_argument);

    t1 = tUnit;
    t2 = tUnit;
    s = mostGeneralUnifier(t1, t2);
    EXPECT_EQ(s.size(), 0);

    t2 = tInt;
    EXPECT_THROW(mostGeneralUnifier(t1, t2), std::invalid_argument);

    t1 = std::make_shared<const TypeVariable>("a", kStar);
    t2 = std::make_shared<const TypeVariable>("b", kStar);
    s = mostGeneralUnifier(t1, t2);
    EXPECT_EQ(s.size(), 1);
    EXPECT_TRUE(sameType(s["a"], t2));

    t1 = std::make_shared<const TypeVariable>("a", kStar);
    t2 = tDouble;
    s = mostGeneralUnifier(t1, t2);
    EXPECT_EQ(s.size(), 1);
    EXPECT_TRUE(sameType(s["a"], t2));

    t1 = tDouble;
    t2 = std::make_shared<const TypeVariable>("a", kStar);
    s = mostGeneralUnifier(t1, t2);
    EXPECT_EQ(s.size(), 1);
    EXPECT_TRUE(sameType(s["a"], t1));

    t1 = std::make_shared<const TypeApplication>(tList, tInt);
    t2 = std::make_shared<const TypeApplication>(tList, std::make_shared<const TypeVariable>("a", kStar));
    s = mostGeneralUnifier(t1, t2);
    EXPECT_EQ(s.size(), 1);
    EXPECT_TRUE(sameType(s["a"], tInt));

    t1 = std::make_shared<const TypeApplication>(tList, tInt);
    t2 = std::make_shared<const TypeApplication>(tList, tInt);
    s = mostGeneralUnifier(t1, t2);
    EXPECT_EQ(s.size(), 0);

    t1 = std::make_shared<const TypeApplication>(tList, tInt);
    t2 = std::make_shared<const TypeApplication>(tList, tDouble);
    EXPECT_THROW(mostGeneralUnifier(t1, t2), std::invalid_argument);
}

TEST(Types, Matching) {
    type t1 = std::make_shared<const TypeVariable>("a", kStar);
    type t2 = std::make_shared<const TypeVariable>("a", kStar);
    substitution s = match(t1, t2);
    EXPECT_EQ(s.size(), 0);

    t2 = std::make_shared<const TypeVariable>("a", kStarToStar);
    EXPECT_THROW(match(t1, t2), std::invalid_argument);

    t2 = std::make_shared<const TypeApplication>(tList, t1);
    s = match(t1, t2);
    EXPECT_EQ(s.size(), 1);
    EXPECT_TRUE(sameType(s["a"], t2));

    t1 = tUnit;
    t2 = tUnit;
    s = match(t1, t2);
    EXPECT_EQ(s.size(), 0);

    t2 = tInt;
    EXPECT_THROW(match(t1, t2), std::invalid_argument);

    t1 = std::make_shared<const TypeVariable>("a", kStar);
    t2 = std::make_shared<const TypeVariable>("b", kStar);
    s = match(t1, t2);
    EXPECT_EQ(s.size(), 1);
    EXPECT_TRUE(sameType(s["a"], t2));

    t1 = std::make_shared<const TypeVariable>("a", kStar);
    t2 = tDouble;
    s = match(t1, t2);
    EXPECT_EQ(s.size(), 1);
    EXPECT_TRUE(sameType(s["a"], t2));

    t1 = tDouble;
    t2 = std::make_shared<const TypeVariable>("a", kStar);
    EXPECT_THROW(match(t1, t2), std::invalid_argument);

    t1 = std::make_shared<const TypeApplication>(tList, tInt);
    t2 = std::make_shared<const TypeApplication>(tList, std::make_shared<const TypeVariable>("a", kStar));
    EXPECT_THROW(match(t1, t2), std::invalid_argument);

    t1 = std::make_shared<const TypeApplication>(tList, std::make_shared<const TypeVariable>("a", kStar));
    t2 = std::make_shared<const TypeApplication>(tList, tInt);
    s = match(t1, t2);
    EXPECT_EQ(s.size(), 1);
    EXPECT_TRUE(sameType(s["a"], tInt));

    t1 = std::make_shared<const TypeApplication>(tList, tInt);
    t2 = std::make_shared<const TypeApplication>(tList, tInt);
    s = match(t1, t2);
    EXPECT_EQ(s.size(), 0);

    t1 = std::make_shared<const TypeApplication>(tList, tInt);
    t2 = std::make_shared<const TypeApplication>(tList, tDouble);
    EXPECT_THROW(match(t1, t2), std::invalid_argument);
}

TEST(Types, Scheme) {
    std::shared_ptr<const TypeVariable> a = std::make_shared<const TypeVariable>("a", kStar);
    type b = std::make_shared<const TypeVariable>("b", kStar);
    std::shared_ptr<const TypeVariable> c = std::make_shared<const TypeVariable>("c", kStarToStar);
    type t = std::make_shared<const TypeApplication>(
            std::make_shared<const TypeApplication>(tTuple2, a),
            std::make_shared<const TypeApplication>(
                    std::make_shared<const TypeApplication>(tTuple2, b),
                    std::make_shared<const TypeApplication>(c, a)
            )
    );
    std::vector<std::shared_ptr<const TypeVariable>> vs = {a, c};
    Scheme s(vs, t);
    type a1 = std::make_shared<const TypeGeneric>(0, kStar);
    type c1 = std::make_shared<const TypeGeneric>(1, kStarToStar);
    type t1 = std::make_shared<const TypeApplication>(
            std::make_shared<const TypeApplication>(tTuple2, a1),
            std::make_shared<const TypeApplication>(
                    std::make_shared<const TypeApplication>(tTuple2, b),
                    std::make_shared<const TypeApplication>(c1, a1)
            )
    );
    EXPECT_TRUE(sameType(s.getType(), t1));
}

TEST(Types, Assumptions) {
    Assumptions a;
    Scheme s1(std::make_shared<TypeVariable>("a", kStar));
    Scheme s2(tDouble);

    Assumptions b = a.add("vara", s1);
    Assumptions c = b.add("d", s2);

    auto variables = c.findTypeVariables();
    EXPECT_EQ(variables.size(), 1);
    EXPECT_EQ(variables[0], "a");

    EXPECT_EQ(c.find("vara"), s1);
    EXPECT_EQ(c.find("d"), s2);

    substitution s;
    s["a"] = tUnit;
    Scheme s3(tUnit);
    Assumptions d = c.applySubstitution(s);
    variables = d.findTypeVariables();
    EXPECT_EQ(variables.size(), 0);
    EXPECT_EQ(d.find("vara"), s3);
    EXPECT_EQ(d.find("d"), s2);
}
