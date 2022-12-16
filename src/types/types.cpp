#include "types/types.hpp"
#include <string>
#include <algorithm>
#include <stdexcept>
//
//Scheme::Scheme(const std::vector<std::shared_ptr<const TypeVariable>> &variables, const type &tp) {
//    substitution s;
//    int i = 0;
//
//    for (const auto &variable: ::findTypeVariables(tp)) {
//        auto p = [&variable](const std::shared_ptr<const TypeVariable> &v) { return v->getId() == variable; };
//        auto iterator = std::find_if(variables.begin(), variables.end(), p);
//        if (iterator != variables.end()) {
//            s[variable] = std::make_shared<TypeGeneric>(i++, iterator->get()->getKind());
//        }
//    }
//
//    t = ::applySubstitution(tp, s);
//}
//
//Scheme::Scheme(type t): t(t) {}
//
//Scheme Scheme::applySubstitution(const substitution &s) const {
//    return Scheme(::applySubstitution(t, s));
//}
//
//std::vector<std::string> Scheme::findTypeVariables() const {
//    return ::findTypeVariables(t);
//}
//
//type Scheme::getType() const {
//    return t;
//}
//
//bool operator==(const Scheme &lhs, const Scheme &rhs) {
//    return same_type(lhs.getType(), rhs.getType());
//}
//
//bool operator!=(const Scheme &lhs, const Scheme &rhs) {
//    return !(lhs == rhs);
//}
//
//Assumptions::Assumptions(const std::map<std::string, Scheme> &assumptions): assumptions(assumptions) {}
//
//Assumptions Assumptions::add(const std::string &v, const Scheme &scheme) const {
//    auto newAssumptions = assumptions;
//    newAssumptions.insert({v, scheme});
//    return Assumptions(newAssumptions);
//}
//
//Assumptions Assumptions::applySubstitution(const substitution &s) const {
//    std::map<std::string, Scheme> newAssumptions;
//    for (const auto& [v, scheme] : assumptions) {
//        newAssumptions.insert({v, scheme.applySubstitution(s)});
//    }
//    return Assumptions(newAssumptions);
//}
//
//std::vector<std::string> Assumptions::findTypeVariables() const {
//    std::vector<std::string> variables;
//    for (const auto& [v, scheme] : assumptions) {
//        auto moreVariables = scheme.findTypeVariables();
//        for (const auto &v: moreVariables) {
//            if (std::count(variables.begin(), variables.end(), v) == 0) {
//                variables.push_back(v);
//            }
//        }
//    }
//    return variables;
//}
//
//Scheme Assumptions::find(const std::string &v) const {
//    auto iterator = assumptions.find(v);
//    if (iterator == assumptions.end()) {
//        throw std::invalid_argument("Unbound identifier: " + v);
//    }
//    return iterator->second;
//}

type make_function_type(const type &argType, const type &resultType) {
    type partial = std::make_shared<const TypeApplication>(tArrow, argType);
    return std::make_shared<const TypeApplication>(partial, resultType);
}

type make_list_type(const type &elementType) {
    return std::make_shared<const TypeApplication>(tList, elementType);
}

type make_pair_type(const type &leftType, const type &rightType) {
    type partial = std::make_shared<const TypeApplication>(tTuple2, leftType);
    return std::make_shared<const TypeApplication>(partial, rightType);
}

//kind makeTupleConstructorKind(size_t size) {
//    kind k = kStar;
//    for (int i = 0; i < size; i++) {
//        k = std::make_shared<const ArrowKind>(kStar, k);
//    }
//    return k;
//}

type make_tuple_type(const std::vector<type> &components) {
    std::string constructor = "(" + std::string(components.size() - 1, ',') + ")";
    type t = std::make_shared<const TypeConstructor>(constructor);
    for (const auto &c: components) {
        t = std::make_shared<const TypeApplication>(t, c);
    }
    return t;
}

//bool sameKind(const kind &a, const kind &b) {
//    std::shared_ptr<const ArrowKind> arrow1;
//    std::shared_ptr<const ArrowKind> arrow2;
//    switch (a->get_form()) {
//        case kindform::star:
//            return b->get_form() == kindform::star;
//        case kindform::arrow:
//            if (b->get_form() == kindform::arrow) {
//                arrow1 = std::dynamic_pointer_cast<const ArrowKind>(a);
//                arrow2 = std::dynamic_pointer_cast<const ArrowKind>(b);
//                if (sameKind(arrow1->getArg(), arrow2->getArg()) && sameKind(arrow1->getResult(), arrow2->getResult())) {
//                    return true;
//                }
//            }
//            return false;
//    }
//}

bool same_type(const type &a, const type &b) {
    std::shared_ptr<const TypeVariable> variable1;
    std::shared_ptr<const TypeVariable> variable2;
    std::shared_ptr<const TypeConstructor> constructor1;
    std::shared_ptr<const TypeConstructor> constructor2;
    std::shared_ptr<const TypeApplication> application1;
    std::shared_ptr<const TypeApplication> application2;
    if (a->get_form() != b->get_form()) {
        return false;
    }
    switch (a->get_form()) {
        case typeform::variable:
            variable1 = std::dynamic_pointer_cast<const TypeVariable>(a);
            variable2 = std::dynamic_pointer_cast<const TypeVariable>(b);
            return variable1->id == variable2->id;
        case typeform::constructor:
            constructor1 = std::dynamic_pointer_cast<const TypeConstructor>(a);
            constructor2 = std::dynamic_pointer_cast<const TypeConstructor>(b);
            return constructor1->id == constructor2->id;
        case typeform::application:
            application1 = std::dynamic_pointer_cast<const TypeApplication>(a);
            application2 = std::dynamic_pointer_cast<const TypeApplication>(b);
            return same_type(application1->left, application2->left) && same_type(application1->right, application2->right);
    }
}

//type applySubstitution(const type &t, const substitution &s) {
//    std::string id;
//    type left;
//    type newLeft;
//    type right;
//    type newRight;
//    substitution::const_iterator iterator;
//    switch (t->get_form()) {
//        case typeform::variable:
//            id = std::dynamic_pointer_cast<const TypeVariable>(t)->getId();
//            iterator = s.find(id);
//            if (iterator != s.end()) {
//                return iterator->second;
//            }
//            return t;
//        case typeform::constructor:
//            return t;
//        case typeform::application:
//            left = std::dynamic_pointer_cast<const TypeApplication>(t)->getLeft();
//            newLeft = applySubstitution(left, s);
//            right = std::dynamic_pointer_cast<const TypeApplication>(t)->getRight();
//            newRight = applySubstitution(right, s);
//            if (left == newLeft && right == newRight) {
//                return t;
//            }
//            return std::make_shared<TypeApplication>(newLeft, newRight);
//        case typeform::gen:
//            return t;
//    }
//}
//
//std::vector<std::string> findTypeVariables(const type &t) {
//    std::vector<std::string> variables;
//    std::vector<std::string> moreVariables;
//    switch (t->get_form()) {
//        case typeform::variable:
//            variables.push_back(std::dynamic_pointer_cast<const TypeVariable>(t)->getId());
//            return variables;
//        case typeform::constructor:
//            return variables;
//        case typeform::application:
//            variables = findTypeVariables(std::dynamic_pointer_cast<const TypeApplication>(t)->getLeft());
//            moreVariables = findTypeVariables(std::dynamic_pointer_cast<const TypeApplication>(t)->getRight());
//            for (const auto &v: moreVariables) {
//                if (std::count(variables.begin(), variables.end(), v) == 0) {
//                    variables.push_back(v);
//                }
//            }
//            return variables;
//        case typeform::gen:
//            return variables;
//    }
//}
//
//std::vector<type> applySubstitution(const std::vector<type> &ts, const substitution &s) {
//    std::vector<type> result;
//    auto substitute = [&s](const type& t) { return applySubstitution(t, s); };
//    std::transform(ts.cbegin(), ts.cend(), std::back_inserter(result), substitute);
//    return result;
//}
//
//std::vector<std::string> findTypeVariables(const std::vector<type> &ts) {
//    std::vector<std::string> variables;
//    for (const auto& t: ts) {
//        auto moreVariables = findTypeVariables(t);
//        for (const auto &v: moreVariables) {
//            if (std::count(variables.begin(), variables.end(), v) == 0) {
//                variables.push_back(v);
//            }
//        }
//    }
//    return variables;
//}
//
//substitution compose(const substitution &s1, const substitution &s2) {
//    substitution result = s1;
//
//    for (const auto& [k, t] : s2) {
//        result[k] = applySubstitution(t, s1);
//    }
//
//    return result;
//}
//
//substitution merge(const substitution &s1, const substitution &s2) {
//    substitution result = s1;
//
//    for (const auto& [k, t] : s2) {
//        if (result.count(k) > 0) {
//            if (!same_type(result[k], t)) {
//                throw std::invalid_argument("Cannot merge substitutions that do not agree.");
//            }
//        } else {
//            result[k] = t;
//        }
//    }
//    return result;
//}
//
//substitution varBind(const std::shared_ptr<const TypeVariable> &var, const type &t) {
//    auto variables = findTypeVariables(t);
//    if (same_type(var, t)) {
//        substitution empty;
//        return empty;
//    } else if (std::count(variables.begin(), variables.end(), var->getId()) > 0) {
//        throw std::invalid_argument("Occurs check failed.");
//    } else if (!sameKind(var->getKind(), t->getKind())) {
//        throw std::invalid_argument("Kinds do not match.");
//    }
//    substitution s;
//    s[var->getId()] = t;
//    return s;
//}
//
//substitution mostGeneralUnifier(const type &t1, const type &t2) {
//    if (t1->get_form() == typeform::constructor && same_type(t1, t2)) {
//        substitution empty;
//        return empty;
//    } else if (t1->get_form() == typeform::variable) {
//        return varBind(std::dynamic_pointer_cast<const TypeVariable>(t1), t2);
//    } else if (t2->get_form() == typeform::variable) {
//        return varBind(std::dynamic_pointer_cast<const TypeVariable>(t2), t1);
//    } else if (t1->get_form() == typeform::application && t2->get_form() == typeform::application) {
//        const auto ap1 = std::dynamic_pointer_cast<const TypeApplication>(t1);
//        const auto ap2 = std::dynamic_pointer_cast<const TypeApplication>(t2);
//        substitution s1 = mostGeneralUnifier(ap1->getLeft(), ap2->getLeft());
//        substitution s2 = mostGeneralUnifier(applySubstitution(ap1->getRight(), s1), applySubstitution(ap2->getRight(), s1));
//        return compose(s2, s1);
//    }
//    throw std::invalid_argument("Types do not unify.");
//}
//
//substitution match(const type &t1, const type &t2) {
//    if (t1->get_form() == typeform::constructor && same_type(t1, t2)) {
//        substitution empty;
//        return empty;
//    } else if (t1->get_form() == typeform::variable && sameKind(t1->getKind(), t2->getKind())) {
//        if (same_type(t1, t2)) {
//            substitution empty;
//            return empty;
//        }
//        substitution s;
//        s[std::dynamic_pointer_cast<const TypeVariable>(t1)->getId()] = t2;
//        return s;
//    } else if (t1->get_form() == typeform::application && t2->get_form() == typeform::application) {
//        const auto ap1 = std::dynamic_pointer_cast<const TypeApplication>(t1);
//        const auto ap2 = std::dynamic_pointer_cast<const TypeApplication>(t2);
//        substitution s1 = match(ap1->getLeft(), ap2->getLeft());
//        substitution s2 = match(ap1->getRight(), ap2->getRight());
//        return merge(s1, s2);
//    }
//    throw std::invalid_argument("Types do not match.");
//}
