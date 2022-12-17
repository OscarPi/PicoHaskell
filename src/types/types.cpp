#include "types/types.hpp"
#include "parser/syntax.hpp"
#include <string>
#include <algorithm>
#include <stdexcept>

Type *make_function_type(Type* const &argType, Type* const &resultType) {
    Type *partial = new TypeApplication(new TypeConstructor("->"), argType);
    return new TypeApplication(partial, resultType);
}

Type *make_list_type(Type* const &elementType) {
    return new TypeApplication(new TypeConstructor("[]"), elementType);
}


//kind makeTupleConstructorKind(size_t size) {
//    kind k = kStar;
//    for (int i = 0; i < size; i++) {
//        k = std::make_shared<const ArrowKind>(kStar, k);
//    }
//    return k;
//}

Type *make_tuple_type(const std::vector<Type*> &components) {
    std::string constructor = "(" + std::string(components.size() - 1, ',') + ")";
    Type *t = new TypeConstructor(constructor);
    for (const auto &c: components) {
        t = new TypeApplication(t, c);
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

bool same_type(const Type *a, const Type *b) {
    if (a->get_form() != b->get_form()) {
        return false;
    }
    switch (a->get_form()) {
        case typeform::universallyquantifiedvariable:
            return dynamic_cast<const UniversallyQuantifiedVariable*>(a)->id ==
                dynamic_cast<const UniversallyQuantifiedVariable*>(b)->id;
        case typeform::constructor:
            return dynamic_cast<const TypeConstructor*>(a)->id == dynamic_cast<const TypeConstructor*>(b)->id;
        case typeform::application:
            return same_type(
                           dynamic_cast<const TypeApplication*>(a)->left.get(),
                           dynamic_cast<const TypeApplication*>(b)->left.get()) &&
                   same_type(
                           dynamic_cast<const TypeApplication*>(a)->right.get(),
                           dynamic_cast<const TypeApplication*>(b)->right.get());
    }
}

std::shared_ptr<Type> follow_substitution(std::shared_ptr<Type> t) {
    if (t->get_form() != typeform::variable) {
        return t;
    }
    auto v = std::dynamic_pointer_cast<TypeVariable>(t);
    if (v->bound_to == nullptr) {
        return v;
    }
    return follow_substitution(v->bound_to);
}

bool occurs_check_ok(std::shared_ptr<TypeVariable> v, std::shared_ptr<Type> t) {
    if (t == v) {
        return false;
    }
    switch(t->get_form()) {
        case typeform::variable:
            if (std::dynamic_pointer_cast<TypeVariable>(t)->bound_to == nullptr) {
                return true;
            }
            return occurs_check_ok(v, std::dynamic_pointer_cast<TypeVariable>(t)->bound_to);
        case typeform::constructor:
            return true;
        case typeform::application:
            return occurs_check_ok(v, std::dynamic_pointer_cast<TypeApplication>(t)->left) &&
                   occurs_check_ok(v, std::dynamic_pointer_cast<TypeApplication>(t)->right);
        case typeform::universallyquantifiedvariable:
            throw TypeError("Universally quantified variable should not occur in instantiated type.");
    }
}

void unify(std::shared_ptr<Type> a, std::shared_ptr<Type> b) {
    a = follow_substitution(a);
    b = follow_substitution(b);
    if (a == b) {
        return;
    }
    if (
            a->get_form() == typeform::constructor &&
            b->get_form() == typeform::constructor &&
            std::dynamic_pointer_cast<TypeConstructor>(a)->id ==std::dynamic_pointer_cast<TypeConstructor>(b)->id) {
        return;
    } else if (a->get_form() == typeform::variable) {
        if (!occurs_check_ok(std::dynamic_pointer_cast<TypeVariable>(a), b)) {
            throw TypeError("Failed to unify types: occurs check failed.");
        }
        std::dynamic_pointer_cast<TypeVariable>(a)->bound_to = b;
        return;
    } else if (b->get_form() == typeform::variable) {
        if (!occurs_check_ok(std::dynamic_pointer_cast<TypeVariable>(b), a)) {
            throw TypeError("Failed to unify types: occurs check failed.");
        }
        std::dynamic_pointer_cast<TypeVariable>(b)->bound_to = a;
        return;
    } else if (a->get_form() == typeform::application && b->get_form() == typeform::application) {
        unify(
                std::dynamic_pointer_cast<TypeApplication>(a)->left,
                std::dynamic_pointer_cast<TypeApplication>(b)->left);
        unify(
                std::dynamic_pointer_cast<TypeApplication>(a)->right,
                std::dynamic_pointer_cast<TypeApplication>(b)->right);
        return;
    }

    throw TypeError("Failed to unify types.");
}

std::shared_ptr<Type> instantiate(const std::shared_ptr<Type> &t) {
    switch(t->get_form()) {
        case typeform::variable:
            return t;
        case typeform::universallyquantifiedvariable:
            return std::make_shared<TypeVariable>();
        case typeform::constructor:
            return t;
        case typeform::application: {
            auto left = instantiate(std::dynamic_pointer_cast<TypeApplication>(t)->left);
            auto right = instantiate(std::dynamic_pointer_cast<TypeApplication>(t)->right);
            if (
                    left != std::dynamic_pointer_cast<TypeApplication>(t)->left ||
                    right != std::dynamic_pointer_cast<TypeApplication>(t)->right) {
                return std::make_shared<TypeApplication>(left, right);
            } else {
                return t;
            }
        }
    }
}

std::pair<std::shared_ptr<Type>, std::map<std::string, std::shared_ptr<Type>>> type_inference_pattern(
        const std::map<std::string, std::shared_ptr<Type>> &constructor_types,
        const std::unique_ptr<Pattern> &p);

std::pair<std::vector<std::shared_ptr<Type>>, std::map<std::string, std::shared_ptr<Type>>> type_inference_patterns(
        const std::map<std::string, std::shared_ptr<Type>> &constructor_types,
        const std::vector<std::unique_ptr<Pattern>> &ps) {
    std::vector<std::shared_ptr<Type>> types_matched;
    std::map<std::string, std::shared_ptr<Type>> new_assumptions;

    for (const auto &p: ps) {
        auto inferred = type_inference_pattern(constructor_types, p);
        types_matched.push_back(inferred.first);
        for (auto const &[name, type]: inferred.second) {
            new_assumptions[name] = type;
        }
    }

    return std::make_pair(types_matched, new_assumptions);
}

std::pair<std::shared_ptr<Type>, std::map<std::string, std::shared_ptr<Type>>> type_inference_pattern(
        const std::map<std::string, std::shared_ptr<Type>> &constructor_types,
        const std::unique_ptr<Pattern> &p) {
    std::shared_ptr<Type> type_matched;
    std::map<std::string, std::shared_ptr<Type>> new_assumptions;
    switch (p->getForm()) {
        case patternform::constructor: {
            auto sub_patterns = type_inference_patterns(
                    constructor_types,
                    dynamic_cast<ConstructorPattern *>(p.get())->args);
            type_matched = std::make_shared<TypeVariable>();
            new_assumptions = sub_patterns.second;
            auto expected_constructor_type = type_matched;
            for (int i = sub_patterns.first.size() - 1; i >= 0; i--) {
                expected_constructor_type = std::make_shared<TypeApplication>(
                        std::make_shared<TypeApplication>(
                                std::make_shared<TypeConstructor>("->"),
                                sub_patterns.first[i]),
                        expected_constructor_type);
            }
            if (constructor_types.count(dynamic_cast<ConstructorPattern *>(p.get())->name) == 0) {
                throw TypeError(
                        "Line " +
                        std::to_string(p->line) +
                        ": reference in pattern to undefined data constructor " +
                        dynamic_cast<ConstructorPattern *>(p.get())->name + ".");
            }
            auto constructor_type = instantiate(
                    constructor_types.at(dynamic_cast<ConstructorPattern *>(p.get())->name));
            try {
                unify(constructor_type, expected_constructor_type);
            } catch (const TypeError &e) {
                throw TypeError(
                        "Line " +
                        std::to_string(p->line) +
                        ": could not unify the type of the data constructor " +
                        dynamic_cast<ConstructorPattern *>(p.get())->name +
                        " with the type implied by the pattern it was used in.");
            }
            break;
        }
        case patternform::wild:
            type_matched = std::make_shared<TypeVariable>();
            break;
        case patternform::literal:
            if (std::holds_alternative<int>(dynamic_cast<LiteralPattern*>(p.get())->value)) {
                type_matched = std::make_shared<TypeConstructor>("Int");
            } else if (std::holds_alternative<std::string>(dynamic_cast<LiteralPattern*>(p.get())->value)) {
                type_matched = std::make_shared<TypeApplication>(
                        new TypeConstructor("[]"),
                        new TypeConstructor("Char"));
            } else if (std::holds_alternative<char>(dynamic_cast<LiteralPattern*>(p.get())->value)) {
                type_matched = std::make_shared<TypeConstructor>("Char");
            }
            break;
        case patternform::variable:
            type_matched = std::make_shared<TypeVariable>();
            new_assumptions[dynamic_cast<VariablePattern*>(p.get())->name] = type_matched;
            break;
    }

    for (const auto &v: p->as) {
        new_assumptions[v] = type_matched;
    }

    return std::make_pair(type_matched, new_assumptions);
}

std::shared_ptr<Type> type_inference_expression(
        std::map<std::string, std::shared_ptr<Type>> assumptions,
        const std::unique_ptr<Expression> &expression) {
    switch(expression->getForm()) {
        case expform::literal:
            if (std::holds_alternative<int>(dynamic_cast<Literal*>(expression.get())->value)) {
                return std::make_shared<TypeConstructor>("Int");
            } else if (std::holds_alternative<std::string>(dynamic_cast<Literal*>(expression.get())->value)) {
                return std::make_shared<TypeApplication>(
                        new TypeConstructor("[]"),
                        new TypeConstructor("Char"));
            } else if (std::holds_alternative<char>(dynamic_cast<Literal*>(expression.get())->value)) {
                return std::make_shared<TypeConstructor>("Char");
            }
        case expform::variable:
            if (assumptions.count(dynamic_cast<Variable*>(expression.get())->name) == 0) {
                throw TypeError(
                        "Line " +
                        std::to_string(expression->line) +
                        ": undefined reference to name " +
                        dynamic_cast<Variable*>(expression.get())->name + ".");
            }
            return instantiate(assumptions.at(dynamic_cast<Variable*>(expression.get())->name));
        case expform::constructor:
            if (assumptions.count(dynamic_cast<Constructor*>(expression.get())->name) == 0) {
                throw TypeError(
                        "Line " +
                        std::to_string(expression->line) +
                        ": undefined reference to name " +
                        dynamic_cast<Constructor*>(expression.get())->name + ".");
            }
            return instantiate(assumptions.at(dynamic_cast<Constructor*>(expression.get())->name));
        case expform::abstraction: {
            std::shared_ptr<Type> result_type = std::make_shared<TypeVariable>();
            std::shared_ptr<Type> type = result_type;
            for (int i = dynamic_cast<Abstraction*>(expression.get())->args.size() - 1; i >= 0; i--) {
                std::shared_ptr<Type> arg_type = std::make_shared<TypeVariable>();
                assumptions[dynamic_cast<Abstraction*>(expression.get())->args.at(i)] = arg_type;
                type = std::make_shared<TypeApplication>(
                        std::make_shared<TypeApplication>(
                                std::make_shared<TypeConstructor>("->"),
                                arg_type),
                        type);
            }
            unify(
                    result_type,
                    type_inference_expression(assumptions, dynamic_cast<Abstraction*>(expression.get())->body));
            return result_type;
        }
        case expform::application: {
            std::shared_ptr<Type> left_type = type_inference_expression(
                    assumptions,
                    dynamic_cast<Application*>(expression.get())->left);
            std::shared_ptr<Type> right_type = type_inference_expression(
                    assumptions,
                    dynamic_cast<Application*>(expression.get())->right);
            std::shared_ptr<Type> type = std::make_shared<TypeVariable>();
            std::shared_ptr<Type> expected_left_type = std::make_shared<TypeApplication>(
                    std::make_shared<TypeApplication>(
                            std::make_shared<TypeConstructor>("->"),
                            right_type),
                    type);
            try {
                unify(left_type, expected_left_type);
            } catch (const TypeError &e) {
                throw TypeError(
                        "Line " +
                        std::to_string(expression->line) +
                        ": could not infer type for application.");
            }
            return type;
        }
        case expform::builtinop: {
            std::shared_ptr<Type> left_type;
            if (dynamic_cast<BuiltInOp*>(expression.get())->op != builtinop::negate) {
                left_type = type_inference_expression(
                        assumptions,
                        dynamic_cast<BuiltInOp *>(expression.get())->left);
            }
            std::shared_ptr<Type> right_type = type_inference_expression(
                    assumptions,
                    dynamic_cast<BuiltInOp*>(expression.get())->right);
            switch(dynamic_cast<BuiltInOp*>(expression.get())->op) {
                case builtinop::add:
                case builtinop::subtract:
                case builtinop::times:
                case builtinop::divide:
                    try {
                        unify(left_type, std::make_shared<TypeConstructor>("Int"));
                        unify(right_type, std::make_shared<TypeConstructor>("Int"));
                        return std::make_shared<TypeConstructor>("Int");
                    } catch (const TypeError &e) {
                        throw TypeError(
                                "Line " +
                                std::to_string(expression->line) +
                                ": invalid arguments to built in operator.");
                    }
                case builtinop::negate:
                    try {
                        unify(right_type, std::make_shared<TypeConstructor>("Int"));
                        return std::make_shared<TypeConstructor>("Int");
                    } catch (const TypeError &e) {
                        throw TypeError(
                                "Line " +
                                std::to_string(expression->line) +
                                ": invalid arguments to built in operator.");
                    }
                case builtinop::equality:
                case builtinop::inequality:
                    try {
                        unify(left_type, right_type);
                        return std::make_shared<TypeConstructor>("Bool");
                    } catch (const TypeError &e) {
                        throw TypeError(
                                "Line " +
                                std::to_string(expression->line) +
                                ": invalid arguments to built in operator.");
                    }
                case builtinop::lt:
                case builtinop::lte:
                case builtinop::gt:
                case builtinop::gte:
                    try {
                        unify(left_type, std::make_shared<TypeConstructor>("Int"));
                        unify(right_type, std::make_shared<TypeConstructor>("Int"));
                        return std::make_shared<TypeConstructor>("Bool");
                    } catch (const TypeError &e) {
                        throw TypeError(
                                "Line " +
                                std::to_string(expression->line) +
                                ": invalid arguments to built in operator.");
                    }
                case builtinop::land:
                case builtinop::lor:
                    try {
                        unify(left_type, std::make_shared<TypeConstructor>("Bool"));
                        unify(right_type, std::make_shared<TypeConstructor>("Bool"));
                        return std::make_shared<TypeConstructor>("Bool");
                    } catch (const TypeError &e) {
                        throw TypeError(
                                "Line " +
                                std::to_string(expression->line) +
                                ": invalid arguments to built in operator.");
                    }
            }
        }
        case expform::cAsE: {
            std::shared_ptr<Type> exp_type = type_inference_expression(
                    assumptions,
                    dynamic_cast<Case*>(expression.get())->exp);
            std::shared_ptr<Type> result_type = std::make_shared<TypeVariable>();
            for (const auto &alt: dynamic_cast<Case*>(expression.get())->alts) {
                auto pattern = type_inference_pattern(assumptions, alt.first);
                try {
                    unify(pattern.first, exp_type);
                } catch (const TypeError &e) {
                    throw TypeError(
                            "Line " +
                            std::to_string(alt.first->line) +
                            ": type expected by pattern does not unify with type of expression being analysed by case.");
                }
                std::map<std::string, std::shared_ptr<Type>> new_assumptions = assumptions;
                for (const auto &[name, type]: pattern.second) {
                    new_assumptions[name] = type;
                }
                try {
                    unify(
                            result_type,
                            type_inference_expression(new_assumptions, alt.second));
                } catch (const TypeError &e) {
                    throw TypeError(
                            "Line " +
                            std::to_string(alt.first->line) +
                            ": type of expression in alternative does not unify with the types in other alternatives.");
                }
            }
            return result_type;
        }
    }
}

std::shared_ptr<Type> type_inference_declarations(
        std::map<std::string, std::shared_ptr<Type>> assumptions,
        const std::map<std::string, std::unique_ptr<Expression>> &declarations,
        std::map<std::string, std::shared_ptr<Type>> type_signatures
        ) {

}

void typecheck(const std::unique_ptr<Program> &program) {

}
