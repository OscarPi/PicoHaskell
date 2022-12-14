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

TypeGeneric::TypeGeneric(int n, kind k): n(n), k(k) {}

ttype TypeGeneric::getType() const {
    return ttype::gen;
}

int TypeGeneric::getN() const {
    return n;
}

kind TypeGeneric::getKind() const {
    return k;
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

Scheme::Scheme(const std::vector<std::shared_ptr<const TypeVariable>> &variables, const type &tp) {
    substitution s;
    int i = 0;

    for (const auto &variable: ::findTypeVariables(tp)) {
        auto p = [&variable](const std::shared_ptr<const TypeVariable> &v) { return v->getId() == variable; };
        auto iterator = std::find_if(variables.begin(), variables.end(), p);
        if (iterator != variables.end()) {
            s[variable] = std::make_shared<TypeGeneric>(i++, iterator->get()->getKind());
        }
    }

    t = ::applySubstitution(tp, s);
}

Scheme::Scheme(type t): t(t) {}

Scheme Scheme::applySubstitution(const substitution &s) const {
    return Scheme(::applySubstitution(t, s));
}

std::vector<std::string> Scheme::findTypeVariables() const {
    return ::findTypeVariables(t);
}

type Scheme::getType() const {
    return t;
}

bool operator==(const Scheme &lhs, const Scheme &rhs) {
    return sameType(lhs.getType(), rhs.getType());
}

bool operator!=(const Scheme &lhs, const Scheme &rhs) {
    return !(lhs == rhs);
}

Assumptions::Assumptions(const std::map<std::string, Scheme> &assumptions): assumptions(assumptions) {}

Assumptions Assumptions::add(const std::string &v, const Scheme &scheme) const {
    auto newAssumptions = assumptions;
    newAssumptions.insert({v, scheme});
    return Assumptions(newAssumptions);
}

Assumptions Assumptions::applySubstitution(const substitution &s) const {
    std::map<std::string, Scheme> newAssumptions;
    for (const auto& [v, scheme] : assumptions) {
        newAssumptions.insert({v, scheme.applySubstitution(s)});
    }
    return Assumptions(newAssumptions);
}

std::vector<std::string> Assumptions::findTypeVariables() const {
    std::vector<std::string> variables;
    for (const auto& [v, scheme] : assumptions) {
        auto moreVariables = scheme.findTypeVariables();
        for (const auto &v: moreVariables) {
            if (std::count(variables.begin(), variables.end(), v) == 0) {
                variables.push_back(v);
            }
        }
    }
    return variables;
}

Scheme Assumptions::find(const std::string &v) const {
    auto iterator = assumptions.find(v);
    if (iterator == assumptions.end()) {
        throw std::invalid_argument("Unbound identifier: " + v);
    }
    return iterator->second;
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

type applySubstitution(const type &t, const substitution &s) {
    std::string id;
    type left;
    type newLeft;
    type right;
    type newRight;
    substitution::const_iterator iterator;
    switch (t->getType()) {
        case ttype::var:
            id = std::dynamic_pointer_cast<const TypeVariable>(t)->getId();
            iterator = s.find(id);
            if (iterator != s.end()) {
                return iterator->second;
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

std::vector<std::string> findTypeVariables(const type &t) {
    std::vector<std::string> variables;
    std::vector<std::string> moreVariables;
    switch (t->getType()) {
        case ttype::var:
            variables.push_back(std::dynamic_pointer_cast<const TypeVariable>(t)->getId());
            return variables;
        case ttype::con:
            return variables;
        case ttype::ap:
            variables = findTypeVariables(std::dynamic_pointer_cast<const TypeApplication>(t)->getLeft());
            moreVariables = findTypeVariables(std::dynamic_pointer_cast<const TypeApplication>(t)->getRight());
            for (const auto &v: moreVariables) {
                if (std::count(variables.begin(), variables.end(), v) == 0) {
                    variables.push_back(v);
                }
            }
            return variables;
        case ttype::gen:
            return variables;
    }
}

std::vector<type> applySubstitution(const std::vector<type> &ts, const substitution &s) {
    std::vector<type> result;
    auto substitute = [&s](const type& t) { return applySubstitution(t, s); };
    std::transform(ts.cbegin(), ts.cend(), std::back_inserter(result), substitute);
    return result;
}

std::vector<std::string> findTypeVariables(const std::vector<type> &ts) {
    std::vector<std::string> variables;
    for (const auto& t: ts) {
        auto moreVariables = findTypeVariables(t);
        for (const auto &v: moreVariables) {
            if (std::count(variables.begin(), variables.end(), v) == 0) {
                variables.push_back(v);
            }
        }
    }
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

substitution varBind(const std::shared_ptr<const TypeVariable> &var, const type &t) {
    auto variables = findTypeVariables(t);
    if (sameType(var, t)) {
        substitution empty;
        return empty;
    } else if (std::count(variables.begin(), variables.end(), var->getId()) > 0) {
        throw std::invalid_argument("Occurs check failed.");
    } else if (!sameKind(var->getKind(), t->getKind())) {
        throw std::invalid_argument("Kinds do not match.");
    }
    substitution s;
    s[var->getId()] = t;
    return s;
}

substitution mostGeneralUnifier(const type &t1, const type &t2) {
    if (t1->getType() == ttype::con && sameType(t1, t2)) {
        substitution empty;
        return empty;
    } else if (t1->getType() == ttype::var) {
        return varBind(std::dynamic_pointer_cast<const TypeVariable>(t1), t2);
    } else if (t2->getType() == ttype::var) {
        return varBind(std::dynamic_pointer_cast<const TypeVariable>(t2), t1);
    } else if (t1->getType() == ttype::ap && t2->getType() == ttype::ap) {
        const auto ap1 = std::dynamic_pointer_cast<const TypeApplication>(t1);
        const auto ap2 = std::dynamic_pointer_cast<const TypeApplication>(t2);
        substitution s1 = mostGeneralUnifier(ap1->getLeft(), ap2->getLeft());
        substitution s2 = mostGeneralUnifier(applySubstitution(ap1->getRight(), s1), applySubstitution(ap2->getRight(), s1));
        return compose(s2, s1);
    }
    throw std::invalid_argument("Types do not unify.");
}

substitution match(const type &t1, const type &t2) {
    if (t1->getType() == ttype::con && sameType(t1, t2)) {
        substitution empty;
        return empty;
    } else if (t1->getType() == ttype::var && sameKind(t1->getKind(), t2->getKind())) {
        if (sameType(t1, t2)) {
            substitution empty;
            return empty;
        }
        substitution s;
        s[std::dynamic_pointer_cast<const TypeVariable>(t1)->getId()] = t2;
        return s;
    } else if (t1->getType() == ttype::ap && t2->getType() == ttype::ap) {
        const auto ap1 = std::dynamic_pointer_cast<const TypeApplication>(t1);
        const auto ap2 = std::dynamic_pointer_cast<const TypeApplication>(t2);
        substitution s1 = match(ap1->getLeft(), ap2->getLeft());
        substitution s2 = match(ap1->getRight(), ap2->getRight());
        return merge(s1, s2);
    }
    throw std::invalid_argument("Types do not match.");
}
