#include "types/types.hpp"
#include <string>
#include <algorithm>
#include <stdexcept>

TypeVariable::TypeVariable(std::string id, kind k): id(id), k(k) {}

ttype TypeVariable::getType() const {
    return ttype::var;
}

std::string TypeVariable::getId() const {
    return id;
}

kind TypeVariable::getKind() const {
    return k;
}

TypeConstructor::TypeConstructor(std::string id, kind k): id(id), k(k) {}

ttype TypeConstructor::getType() const {
    return ttype::con;
}

std::string TypeConstructor::getId() const {
    return id;
}

kind TypeConstructor::getKind() const {
    return k;
}

TypeApplication::TypeApplication(type left, type right): left(left), right(right) {}

ttype TypeApplication::getType() const {
    return ttype::ap;
}

type TypeApplication::getLeft() const {
    return left;
}

type TypeApplication::getRight() const {
    return right;
}

kind TypeApplication::getKind() const {
    return (*std::dynamic_pointer_cast<const ArrowKind>((*left).getKind())).getResult();
}

TypeGeneric::TypeGeneric(int n): n(n) {}

ttype TypeGeneric::getType() const {
    return ttype::gen;
}

int TypeGeneric::getN() const {
    return n;
}

kind TypeGeneric::getKind() const {
    return nullptr;
}

kindtype StarKind::getType() const {
    return kindtype::star;
}

ArrowKind::ArrowKind(kind arg, kind result): arg(arg), result(result) {}

kindtype ArrowKind::getType() const {
    return kindtype::arrow;
}

kind ArrowKind::getArg() const {
    return arg;
}

kind ArrowKind::getResult() const {
    return result;
}

type makeFunctionType(const type &argType, const type &resultType) {
    type partial = std::make_shared<const TypeApplication>(tArrow, argType);
    return std::make_shared<const TypeApplication>(partial, resultType);
}

type makeListType(const type &elementType) {
    return std::make_shared<const TypeApplication>(tList, elementType);
}

type makePairType(const type &leftType, const type &rightType) {
    type partial = std::make_shared<const TypeApplication>(tTuple2, leftType);
    return std::make_shared<const TypeApplication>(partial, rightType);
}

bool sameKind(const kind &a, const kind &b) {
    std::shared_ptr<const ArrowKind> arrow1;
    std::shared_ptr<const ArrowKind> arrow2;
    switch (a->getType()) {
        case kindtype::star:
            return b->getType() == kindtype::star;
        case kindtype::arrow:
            if (b->getType() == kindtype::arrow) {
                arrow1 = std::dynamic_pointer_cast<const ArrowKind>(a);
                arrow2 = std::dynamic_pointer_cast<const ArrowKind>(b);
                if (sameKind(arrow1->getArg(), arrow2->getArg()) && sameKind(arrow1->getResult(), arrow2->getResult())) {
                    return true;
                }
            }
            return false;
    }
}

bool sameType(const type &a, const type &b) {
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
            return sameKind(variable1->getKind(), variable2->getKind()) && variable1->getId() == variable2->getId();
        case ttype::con:
            constructor1 = std::dynamic_pointer_cast<const TypeConstructor>(a);
            constructor2 = std::dynamic_pointer_cast<const TypeConstructor>(b);
            return sameKind(constructor1->getKind(), constructor2->getKind()) && constructor1->getId() == constructor2->getId();
        case ttype::ap:
            application1 = std::dynamic_pointer_cast<const TypeApplication>(a);
            application2 = std::dynamic_pointer_cast<const TypeApplication>(b);
            return sameType(application1->getLeft(), application2->getLeft()) && sameType(application1->getRight(), application2->getRight());
        case ttype::gen:
            return std::dynamic_pointer_cast<const TypeGeneric>(a)->getN() == std::dynamic_pointer_cast<const TypeGeneric>(b)->getN();
    }
}

type applySubstitution(const type &t, substitution s) {
    std::string id;
    type left;
    type newLeft;
    type right;
    type newRight;
    switch (t->getType()) {
        case ttype::var:
            id = std::dynamic_pointer_cast<const TypeVariable>(t)->getId();
            if (s.count(id) > 0) {
                return s[id];
            }
            return t;
        case ttype::con:
            return t;
        case ttype::ap:
            left = std::dynamic_pointer_cast<const TypeApplication>(t)->getLeft();
            newLeft = applySubstitution(left, s);
            right = std::dynamic_pointer_cast<const TypeApplication>(t)->getRight();
            newRight = applySubstitution(right, s);
            if (left == newLeft && right == newRight) {
                return t;
            }
            return std::make_shared<TypeApplication>(newLeft, newRight);
        case ttype::gen:
            return t;
    }
}

std::set<std::string> findTypeVariables(const type &t) {
    std::set<std::string> variables;
    std::set<std::string> moreVariables;
    switch (t->getType()) {
        case ttype::var:
            variables.insert(std::dynamic_pointer_cast<const TypeVariable>(t)->getId());
            return variables;
        case ttype::con:
            return variables;
        case ttype::ap:
            variables = findTypeVariables(std::dynamic_pointer_cast<const TypeApplication>(t)->getLeft());
            moreVariables = findTypeVariables(std::dynamic_pointer_cast<const TypeApplication>(t)->getRight());
            variables.insert(moreVariables.begin(), moreVariables.end());
            return variables;
        case ttype::gen:
            return variables;
    }
}

std::vector<type> applySubstitution(const std::vector<type> &ts, substitution s) {
    std::vector<type> result;
    auto substitute = [&s](const type& t) { return applySubstitution(t, s); };
    std::transform(ts.cbegin(), ts.cend(), std::back_inserter(result), substitute);
    return result;
}

std::set<std::string> findTypeVariables(const std::vector<type> &ts) {
    std::set<std::string> variables;
    auto find = [&variables](const type& t) {
        auto moreVariables = findTypeVariables(t);
        variables.insert(moreVariables.begin(), moreVariables.end());
    };
    std::for_each(ts.cbegin(), ts.cend(), find);
    return variables;
}

substitution compose(const substitution &s1, const substitution &s2) {
    substitution result = s1;

    for (const auto& [k, t] : s2) {
        result[k] = applySubstitution(t, s1);
    }

    return result;
}

substitution merge(const substitution &s1, const substitution &s2) {
    substitution result = s1;

    for (const auto& [k, t] : s2) {
        if (result.count(k) > 0) {
            if (!sameType(result[k], t)) {
                throw std::invalid_argument("Cannot merge substitutions that do not agree.");
            }
        } else {
            result[k] = t;
        }
    }

    return result;
}
