#include "types/types.hpp"
#include "parser/syntax.hpp"
#include <string>
#include <algorithm>


Type *make_function_type(Type* const &argType, Type* const &resultType) {
    Type *partial = new TypeApplication(new TypeConstructor("->"), argType);
    return new TypeApplication(partial, resultType);
}

Type *make_list_type(Type* const &elementType) {
    return new TypeApplication(new TypeConstructor("[]"), elementType);
}

Type *make_tuple_type(const std::vector<Type*> &components) {
    std::string constructor = "(" + std::string(components.size() - 1, ',') + ")";
    Type *t = new TypeConstructor(constructor);
    for (const auto &c: components) {
        t = new TypeApplication(t, c);
    }
    return t;
}

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

enum class kindform {star, arrow, variable};

struct Kind {
    virtual ~Kind() = default;
    virtual kindform get_form() const = 0;
};

struct StarKind : public Kind {
    kindform get_form() const override { return kindform::star; }
};

struct ArrowKind : public Kind {
    const std::shared_ptr<Kind> left;
    const std::shared_ptr<Kind> right;
    kindform get_form() const override { return kindform::arrow; }
    ArrowKind(
            const std::shared_ptr<Kind> &left,
            const std::shared_ptr<Kind> &right): left(left), right(right) {}
};

struct KindVariable : public Kind {
    std::shared_ptr<Kind> bound_to;
    kindform get_form() const override { return kindform::variable; }
};

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

bool occurs_check_ok(const std::shared_ptr<TypeVariable> &v, const std::shared_ptr<Type> &t) {
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
            std::dynamic_pointer_cast<TypeConstructor>(a)->id == std::dynamic_pointer_cast<TypeConstructor>(b)->id) {
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

void match(std::shared_ptr<Type> a, std::shared_ptr<Type> b) {
    a = follow_substitution(a);
    b = follow_substitution(b);
    if (a == b) {
        return;
    }
    if (
            a->get_form() == typeform::constructor &&
            b->get_form() == typeform::constructor &&
            std::dynamic_pointer_cast<TypeConstructor>(a)->id == std::dynamic_pointer_cast<TypeConstructor>(b)->id) {
        return;
    } else if (a->get_form() == typeform::variable) {
        if (!occurs_check_ok(std::dynamic_pointer_cast<TypeVariable>(a), b)) {
            throw TypeError("Failed to match types: occurs check failed.");
        }
        std::dynamic_pointer_cast<TypeVariable>(a)->bound_to = b;
        return;
    } else if (a->get_form() == typeform::application && b->get_form() == typeform::application) {
        match(
                std::dynamic_pointer_cast<TypeApplication>(a)->left,
                std::dynamic_pointer_cast<TypeApplication>(b)->left);
        match(
                std::dynamic_pointer_cast<TypeApplication>(a)->right,
                std::dynamic_pointer_cast<TypeApplication>(b)->right);
        return;
    }

    throw TypeError("Failed to match types.");
}

std::shared_ptr<Type> instantiate(
        const std::shared_ptr<Type> &t,
        std::map<std::string, std::shared_ptr<Type>> &variables) {
    switch(t->get_form()) {
        case typeform::variable:
            return t;
        case typeform::universallyquantifiedvariable: {
            const std::string name = std::dynamic_pointer_cast<UniversallyQuantifiedVariable>(t)->id;
            if (variables.count(name) == 0) {
                variables[name] = std::make_shared<TypeVariable>();
            }
            return variables.at(name);
        }
        case typeform::constructor:
            return t;
        case typeform::application: {
            auto left = instantiate(std::dynamic_pointer_cast<TypeApplication>(t)->left, variables);
            auto right = instantiate(std::dynamic_pointer_cast<TypeApplication>(t)->right, variables);
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

std::shared_ptr<Type> instantiate(const std::shared_ptr<Type> &t) {
    std::map<std::string, std::shared_ptr<Type>> variables;
    return instantiate(t, variables);
}

std::shared_ptr<Type> generalise(
        const std::shared_ptr<Type> &t,
        const std::map<std::string, std::shared_ptr<Type>> &assumptions) {
    std::shared_ptr<Type> type = follow_substitution(t);
    switch(type->get_form()) {
        case typeform::variable: {
            for (const auto &[name, t1]: assumptions) {
                if (!occurs_check_ok(std::dynamic_pointer_cast<TypeVariable>(type), t1)) {
                    return type;
                }
            }
            return std::make_shared<UniversallyQuantifiedVariable>(std::dynamic_pointer_cast<TypeVariable>(type)->id);
        }
        case typeform::universallyquantifiedvariable:
        case typeform::constructor:
            return type;
        case typeform::application:
            auto left = generalise(std::dynamic_pointer_cast<TypeApplication>(type)->left, assumptions);
            auto right = generalise(std::dynamic_pointer_cast<TypeApplication>(type)->right, assumptions);
            if (
                    left != std::dynamic_pointer_cast<TypeApplication>(type)->left ||
                    right != std::dynamic_pointer_cast<TypeApplication>(type)->right) {
                return std::make_shared<TypeApplication>(left, right);
            } else {
                return type;
            }
    }
}

std::vector<std::string> get_variables_bound_by(const std::unique_ptr<Pattern> &pattern) {
    std::vector<std::string> variables = pattern->as;
    switch(pattern->getForm()) {
        case patternform::wild:
        case patternform::literal:
            return variables;
        case patternform::variable:
            variables.push_back(dynamic_cast<VariablePattern*>(pattern.get())->name);
            return variables;
        case patternform::constructor:
            for (const auto &pat: dynamic_cast<ConstructorPattern*>(pattern.get())->args) {
                std::vector<std::string> new_variables = get_variables_bound_by(pat);
                variables.insert(variables.end(), new_variables.begin(), new_variables.end());
            }
            return variables;
    }
}

std::pair<std::shared_ptr<Type>, std::map<std::string, std::shared_ptr<Type>>> type_inference_pattern(
        const std::map<std::string, std::shared_ptr<Type>> &constructor_types,
        const std::map<std::string, int> &constructor_arities,
        const std::unique_ptr<Pattern> &p);

std::pair<std::vector<std::shared_ptr<Type>>, std::map<std::string, std::shared_ptr<Type>>> type_inference_patterns(
        const std::map<std::string, std::shared_ptr<Type>> &constructor_types,
        const std::map<std::string, int> &constructor_arities,
        const std::vector<std::unique_ptr<Pattern>> &ps) {
    std::vector<std::shared_ptr<Type>> types_matched;
    std::map<std::string, std::shared_ptr<Type>> new_assumptions;

    for (const auto &p: ps) {
        auto inferred = type_inference_pattern(constructor_types, constructor_arities, p);
        types_matched.push_back(inferred.first);
        for (auto const &[name, type]: inferred.second) {
            new_assumptions[name] = type;
        }
    }

    return std::make_pair(types_matched, new_assumptions);
}

std::pair<std::shared_ptr<Type>, std::map<std::string, std::shared_ptr<Type>>> type_inference_pattern(
        const std::map<std::string, std::shared_ptr<Type>> &constructor_types,
        const std::map<std::string, int> &constructor_arities,
        const std::unique_ptr<Pattern> &p) {
    std::vector<std::string> variables = get_variables_bound_by(p);
    if (variables.size() > std::set(variables.begin(), variables.end()).size()) {
        throw TypeError(
                "Line " +
                std::to_string(p->line) +
                ": a variable should occur at most once within a pattern.");
    }
    std::shared_ptr<Type> type_matched;
    std::map<std::string, std::shared_ptr<Type>> new_assumptions;
    switch (p->getForm()) {
        case patternform::constructor: {
            if (constructor_types.count(dynamic_cast<ConstructorPattern *>(p.get())->name) == 0) {
                throw TypeError(
                        "Line " +
                        std::to_string(p->line) +
                        ": reference in pattern to undefined data constructor " +
                        dynamic_cast<ConstructorPattern *>(p.get())->name + ".");
            }
            if (dynamic_cast<ConstructorPattern *>(p.get())->args.size() !=
                constructor_arities.at(dynamic_cast<ConstructorPattern *>(p.get())->name)) {
                throw TypeError(
                        "Line " +
                        std::to_string(p->line) +
                        ": one cannot match against a partially-applied constructor.");
            }
            auto sub_patterns = type_inference_patterns(
                    constructor_types,
                    constructor_arities,
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

std::map<std::string, std::shared_ptr<Type>> type_inference_declarations(
        const std::map<std::string, std::shared_ptr<Type>> &assumptions,
        const std::map<std::string, int> &constructor_arities,
        const std::map<std::string, std::unique_ptr<Expression>> &declarations,
        const std::map<std::string, std::shared_ptr<Type>> &type_signatures);

std::shared_ptr<Type> type_inference_expression(
        std::map<std::string, std::shared_ptr<Type>> assumptions,
        const std::map<std::string, int> &constructor_arities,
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
                    type_inference_expression(
                            assumptions,
                            constructor_arities,
                            dynamic_cast<Abstraction*>(expression.get())->body));
            return result_type;
        }
        case expform::application: {
            std::shared_ptr<Type> left_type = type_inference_expression(
                    assumptions,
                    constructor_arities,
                    dynamic_cast<Application*>(expression.get())->left);
            std::shared_ptr<Type> right_type = type_inference_expression(
                    assumptions,
                    constructor_arities,
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
                        constructor_arities,
                        dynamic_cast<BuiltInOp *>(expression.get())->left);
            }
            std::shared_ptr<Type> right_type = type_inference_expression(
                    assumptions,
                    constructor_arities,
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
                    constructor_arities,
                    dynamic_cast<Case*>(expression.get())->exp);
            std::shared_ptr<Type> result_type = std::make_shared<TypeVariable>();
            for (const auto &alt: dynamic_cast<Case*>(expression.get())->alts) {
                auto pattern = type_inference_pattern(assumptions, constructor_arities, alt.first);
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
                            type_inference_expression(new_assumptions, constructor_arities, alt.second));
                } catch (const TypeError &e) {
                    throw TypeError(
                            "Line " +
                            std::to_string(alt.first->line) +
                            ": type of expression in alternative does not unify with the types in other alternatives.");
                }
            }
            return result_type;
        }
        case expform::let: {
            std::map<std::string, std::shared_ptr<Type>> local_assumptions = type_inference_declarations(
                    assumptions,
                    constructor_arities,
                    dynamic_cast<Let*>(expression.get())->bindings,
                    dynamic_cast<Let*>(expression.get())->type_signatures);
            return type_inference_expression(
                    local_assumptions,
                    constructor_arities,
                    dynamic_cast<Let*>(expression.get())->e);
        }
    }
}

std::set<std::string> get_free_variables(const std::unique_ptr<Expression> &exp) {
    switch(exp->getForm()) {
        case expform::variable:
            return std::set<std::string>({dynamic_cast<Variable*>(exp.get())->name});
        case expform::constructor:
        case expform::literal:
            return {};
        case expform::abstraction: {
            std::set<std::string> variables = get_free_variables(dynamic_cast<Abstraction*>(exp.get())->body);
            for (const std::string &v: dynamic_cast<Abstraction*>(exp.get())->args) {
                variables.erase(v);
            }
            return variables;
        }
        case expform::application: {
            std::set<std::string> variables = get_free_variables(dynamic_cast<Application*>(exp.get())->left);
            for (const std::string &v: get_free_variables(dynamic_cast<Application*>(exp.get())->right)) {
                variables.insert(v);
            }
            return variables;
        }
        case expform::cAsE: {
            std::set<std::string> variables = get_free_variables(dynamic_cast<Case*>(exp.get())->exp);
            for (const auto &alt: dynamic_cast<Case*>(exp.get())->alts) {
                std::set<std::string> new_variables = get_free_variables(alt.second);
                for (const std::string &v: get_variables_bound_by(alt.first)) {
                    new_variables.erase(v);
                }
                for (const std::string &v: new_variables) {
                    variables.insert(v);
                }
            }
            return variables;
        }
        case expform::let: {
            std::set<std::string> variables = get_free_variables(dynamic_cast<Let*>(exp.get())->e);
            std::set<std::string> bound_names;
            for (const auto &[name, e]: dynamic_cast<Let*>(exp.get())->bindings) {
                bound_names.insert(name);
                for (const std::string &v: get_free_variables(e)) {
                    variables.insert(v);
                }
            }
            for (const std::string &v: bound_names) {
                variables.erase(v);
            }
            return variables;
        }
        case expform::builtinop: {
            std::set<std::string> variables = get_free_variables(dynamic_cast<BuiltInOp*>(exp.get())->left);
            for (const std::string &v: get_free_variables(dynamic_cast<BuiltInOp*>(exp.get())->right)) {
                variables.insert(v);
            }
            return variables;
        }
    }
}

std::map<std::string, std::shared_ptr<Type>> type_inference_declarations(
        const std::map<std::string, std::shared_ptr<Type>> &assumptions,
        const std::map<std::string, int> &constructor_arities,
        const std::map<std::string, std::unique_ptr<Expression>> &declarations,
        const std::map<std::string, std::shared_ptr<Type>> &type_signatures) {
    std::map<std::string, std::shared_ptr<Type>> local_assumptions = assumptions;

    std::map<std::string, std::set<std::string>> free_variables;

    std::vector<std::string> explicitly_typed_bindings;
    std::vector<std::string> implicitly_typed_bindings;

    for (const auto &[name, _]: type_signatures) {
        if (declarations.count(name) == 0) {
            throw TypeError(
                    "Type signature for " +
                    name +
                    " with no matching binding.");
        }
    }

    for (const auto &[name, definition]: declarations) {
        if (type_signatures.count(name) > 0) {
            explicitly_typed_bindings.push_back(name);
        } else {
            implicitly_typed_bindings.push_back(name);
            free_variables[name] = get_free_variables(definition);
        }
    }

    for (const auto &name: explicitly_typed_bindings) {
        local_assumptions[name] = type_signatures.at(name);
    }

    std::vector<std::vector<std::string>> to_typecheck;
    while (!implicitly_typed_bindings.empty()) {
        to_typecheck.push_back({implicitly_typed_bindings.back()});
        implicitly_typed_bindings.pop_back();

        while (!to_typecheck.empty()) {
            std::vector<std::string> current = to_typecheck.back();
            to_typecheck.pop_back();

            std::vector<std::string> depends_on;
            for (const auto &name: current) {
                for (const auto &dependency: free_variables[name]) {
                    depends_on.push_back(dependency);
                }
            }

            bool dependencies_ok = true;
            for (const auto &dependency: depends_on) {
                if (std::count(implicitly_typed_bindings.begin(), implicitly_typed_bindings.end(), dependency) > 0) {
                    to_typecheck.push_back(current);
                    to_typecheck.push_back({dependency});
                    implicitly_typed_bindings.erase(
                            std::find(
                                    implicitly_typed_bindings.begin(),
                                    implicitly_typed_bindings.end(),
                                    dependency));
                    dependencies_ok = false;
                    break;
                } else {
                    for (int i = 0; i < to_typecheck.size(); i++) {
                        if (std::count(to_typecheck[i].begin(), to_typecheck[i].end(), dependency) > 0) {
                            size_t number_to_pop = to_typecheck.size() - i;
                            std::vector<std::string> new_group;
                            for (int j = 0; j < number_to_pop; j++) {
                                new_group.insert(new_group.end(), to_typecheck.back().begin(), to_typecheck.back().end());
                                to_typecheck.pop_back();
                            }
                            new_group.insert(new_group.end(), current.begin(), current.end());
                            dependencies_ok = false;
                            break;
                        }
                    }
                    if (!dependencies_ok) {
                        break;
                    }
                }
            }

            if (dependencies_ok) {
                for (const auto &name: current) {
                    local_assumptions[name] = std::make_shared<TypeVariable>();
                }
                for (const auto &name: current) {
                    std::shared_ptr<Type> type = type_inference_expression(
                            local_assumptions,
                            constructor_arities,
                            declarations.at(name));
                    try {
                        unify(type, local_assumptions[name]);
                    } catch (const TypeError &e) {
                        throw TypeError(
                                "Line " +
                                std::to_string(declarations.at(name)->line) +
                                ": could not deduce type for name " +
                                name + ".");
                    }
                }
                for (const auto &name: current) {
                    local_assumptions[name] = generalise(local_assumptions[name], assumptions);
                }
            }
        }
    }

    for (const auto &name: explicitly_typed_bindings) {
        std::shared_ptr<Type> type = type_inference_expression(
                local_assumptions,
                constructor_arities,
                declarations.at(name));
        try {
            match(type, instantiate(local_assumptions[name]));
        } catch (const TypeError &e) {
            throw TypeError(
                    "Line " +
                    std::to_string(declarations.at(name)->line) +
                    ": could not confirm type for name " +
                    name + ".");
        }
    }

    return local_assumptions;
}

std::shared_ptr<Kind> follow_substitution(std::shared_ptr<Kind> k) {
    if (k->get_form() != kindform::variable) {
        return k;
    }
    auto v = std::dynamic_pointer_cast<KindVariable>(k);
    if (v->bound_to == nullptr) {
        return v;
    }
    return follow_substitution(v->bound_to);
}

bool occurs_check_ok(const std::shared_ptr<KindVariable> &v, const std::shared_ptr<Kind> &k) {
    if (k == v) {
        return false;
    }
    switch(k->get_form()) {
        case kindform::variable:
            if (std::dynamic_pointer_cast<KindVariable>(k)->bound_to == nullptr) {
                return true;
            }
            return occurs_check_ok(v, std::dynamic_pointer_cast<KindVariable>(k)->bound_to);
        case kindform::star:
            return true;
        case kindform::arrow:
            return occurs_check_ok(v, std::dynamic_pointer_cast<ArrowKind>(k)->left) &&
                   occurs_check_ok(v, std::dynamic_pointer_cast<ArrowKind>(k)->right);
    }
}

void unify(std::shared_ptr<Kind> a, std::shared_ptr<Kind> b) {
    a = follow_substitution(a);
    b = follow_substitution(b);
    if (a == b) {
        return;
    }
    if (a->get_form() == kindform::star && b->get_form() == kindform::star) {
        return;
    } else if (a->get_form() == kindform::variable) {
        if (!occurs_check_ok(std::dynamic_pointer_cast<KindVariable>(a), b)) {
            throw TypeError("Failed to unify kinds: occurs check failed.");
        }
        std::dynamic_pointer_cast<KindVariable>(a)->bound_to = b;
        return;
    } else if (b->get_form() == kindform::variable) {
        if (!occurs_check_ok(std::dynamic_pointer_cast<KindVariable>(b), a)) {
            throw TypeError("Failed to unify kinds: occurs check failed.");
        }
        std::dynamic_pointer_cast<KindVariable>(b)->bound_to = a;
        return;
    } else if (a->get_form() == kindform::arrow && b->get_form() == kindform::arrow) {
        unify(
                std::dynamic_pointer_cast<ArrowKind>(a)->left,
                std::dynamic_pointer_cast<ArrowKind>(b)->left);
        unify(
                std::dynamic_pointer_cast<ArrowKind>(a)->right,
                std::dynamic_pointer_cast<ArrowKind>(b)->right);
        return;
    }

    throw TypeError("Failed to unify kinds.");
}

std::shared_ptr<Kind> generalise(const std::shared_ptr<Kind> &k) {
    std::shared_ptr<Kind> kind = follow_substitution(k);
    switch(kind->get_form()) {
        case kindform::star:
            return kind;
        case kindform::variable:
            return std::make_shared<StarKind>();
        case kindform::arrow:
            auto left = generalise(std::dynamic_pointer_cast<ArrowKind>(kind)->left);
            auto right = generalise(std::dynamic_pointer_cast<ArrowKind>(kind)->right);
            if (
                    left != std::dynamic_pointer_cast<ArrowKind>(kind)->left ||
                    right != std::dynamic_pointer_cast<ArrowKind>(kind)->right) {
                return std::make_shared<ArrowKind>(left, right);
            } else {
                return kind;
            }
    }
}

std::shared_ptr<Kind> kind_inference(
        const std::shared_ptr<Type> &t,
        const std::map<std::string, std::shared_ptr<Kind>> &variable_kinds,
        const std::map<std::string, std::shared_ptr<Kind>> &constructor_kinds) {
    switch(t->get_form()) {
        case typeform::variable:
            throw TypeError("cannot infer kind of instantiated type variable.");
        case typeform::universallyquantifiedvariable:
            if (variable_kinds.count(std::dynamic_pointer_cast<Variable>(t)->name) == 0) {
                throw TypeError(
                        "unbound type variable" +
                        std::dynamic_pointer_cast<Variable>(t)->name + ".");
            }
            return variable_kinds.at(std::dynamic_pointer_cast<Variable>(t)->name);
        case typeform::constructor:
            if (constructor_kinds.count(std::dynamic_pointer_cast<TypeConstructor>(t)->id) == 0) {
                throw TypeError(
                        "unbound type constructor " +
                        std::dynamic_pointer_cast<Constructor>(t)->name + ".");
            }
            return constructor_kinds.at(std::dynamic_pointer_cast<TypeConstructor>(t)->id);
        case typeform::application: {
            std::shared_ptr<Kind> left_kind = kind_inference(
                    dynamic_cast<TypeApplication*>(t.get())->left,
                    variable_kinds,
                    constructor_kinds);
            std::shared_ptr<Kind> right_kind = kind_inference(
                    dynamic_cast<TypeApplication*>(t.get())->right,
                    variable_kinds,
                    constructor_kinds);
            std::shared_ptr<Kind> kind = std::make_shared<KindVariable>();
            std::shared_ptr<Kind> expected_left_kind = std::make_shared<ArrowKind>(right_kind, kind);
            try {
                unify(left_kind, expected_left_kind);
            } catch (const TypeError &e) {
                throw TypeError("could not infer kind.");
            }
            return kind;
        }
    }
}

std::set<std::string> get_referenced_type_constructors(std::shared_ptr<Type> t) {
    t = follow_substitution(t);
    switch(t->get_form()) {
        case typeform::universallyquantifiedvariable:
        case typeform::variable:
            return {};
        case typeform::application: {
            std::set<std::string> left = get_referenced_type_constructors(
                    std::dynamic_pointer_cast<TypeApplication>(t)->left);
            std::set<std::string> right = get_referenced_type_constructors(
                    std::dynamic_pointer_cast<TypeApplication>(t)->right);
            left.insert(right.begin(), right.end());
            return left;
        }
        case typeform::constructor:
            return {std::dynamic_pointer_cast<TypeConstructor>(t)->id};
    }
}

void type_check(const std::unique_ptr<Program> &program) {
    std::map<std::string, std::shared_ptr<Type>> assumptions;

    std::map<std::string, int> data_constructor_arities;

    std::map<std::string, std::shared_ptr<Kind>> type_constructor_kinds;
    type_constructor_kinds["Int"] = std::make_shared<StarKind>();
    type_constructor_kinds["Char"] = std::make_shared<StarKind>();
    type_constructor_kinds["Bool"] = std::make_shared<StarKind>();

    std::vector<std::string> type_constructor_names;
    std::map<std::string, std::vector<std::string>> free_type_constructors;

    for (const auto &[name, constructor]: program->type_constructors) {
        type_constructor_names.push_back(name);
        std::vector<std::string> referenced_type_constructors;
        for (const auto &data_constructor: constructor->data_constructors) {
            for (const auto &type: program->data_constructors[data_constructor]->types) {
                for (const auto &n: get_referenced_type_constructors(type)) {
                    referenced_type_constructors.push_back(n);
                }
            }
        }
        free_type_constructors[name] = referenced_type_constructors;
    }

    std::vector<std::vector<std::string>> to_kind_check;
    while (!type_constructor_names.empty()) {
        to_kind_check.push_back({type_constructor_names.back()});
        type_constructor_names.pop_back();

        while (!to_kind_check.empty()) {
            std::vector<std::string> current = to_kind_check.back();
            to_kind_check.pop_back();

            std::vector<std::string> depends_on;
            for (const auto &type_constructor: current) {
                for (const auto &dependency: free_type_constructors[type_constructor]) {
                    depends_on.push_back(dependency);
                }
            }

            bool dependencies_ok = true;
            for (const auto &dependency: depends_on) {
                if (std::count(type_constructor_names.begin(), type_constructor_names.end(), dependency) > 0) {
                    to_kind_check.push_back(current);
                    to_kind_check.push_back({dependency});
                    type_constructor_names.erase(
                            std::find(
                                    type_constructor_names.begin(),
                                    type_constructor_names.end(),
                                    dependency));
                    dependencies_ok = false;
                    break;
                } else {
                    for (int i = 0; i < to_kind_check.size(); i++) {
                        if (std::count(to_kind_check[i].begin(), to_kind_check[i].end(), dependency) > 0) {
                            size_t number_to_pop = to_kind_check.size() - i;
                            std::vector<std::string> new_group;
                            for (int j = 0; j < number_to_pop; j++) {
                                new_group.insert(new_group.end(), to_kind_check.back().begin(), to_kind_check.back().end());
                                to_kind_check.pop_back();
                            }
                            new_group.insert(new_group.end(), current.begin(), current.end());
                            dependencies_ok = false;
                            break;
                        }
                    }
                    if (!dependencies_ok) {
                        break;
                    }
                }
            }

            if (dependencies_ok) {
                std::map<std::string, std::map<std::string, std::shared_ptr<Kind>>> argument_variable_kinds;
                for (const std::string &type_constructor: current) {
                    std::map<std::string, std::shared_ptr<Kind>> arg_kinds;
                    std::shared_ptr<Kind> type_constructor_kind = std::make_shared<StarKind>();
                    for (int i = program->type_constructors.at(type_constructor)->argument_variables.size(); i >= 0; i--) {
                        std::shared_ptr<Kind> k = std::make_shared<KindVariable>();
                        arg_kinds[program->type_constructors.at(type_constructor)->argument_variables[i]] = k;
                        type_constructor_kind = std::make_shared<ArrowKind>(k, type_constructor_kind);
                    }
                    argument_variable_kinds[type_constructor] = arg_kinds;
                    type_constructor_kinds[type_constructor] = type_constructor_kind;
                }
                for (const std::string &type_constructor: current) {
                    std::shared_ptr<Type> base_data_constructor_type = std::make_shared<TypeConstructor>(type_constructor);
                    for (const std::string &variable: program->type_constructors[type_constructor]->argument_variables) {
                        base_data_constructor_type = std::make_shared<TypeApplication>(
                                base_data_constructor_type,
                                std::make_shared<UniversallyQuantifiedVariable>(variable));
                    }
                    for (const std::string &data_constructor: program->type_constructors[type_constructor]->data_constructors) {
                        std::shared_ptr<Type> data_constructor_type = base_data_constructor_type;
                        for (int i = program->data_constructors[data_constructor]->types.size(); i >= 0; i--) {
                            std::shared_ptr<Kind> k;
                            try {
                                k = kind_inference(
                                        program->data_constructors[data_constructor]->types[i],
                                        argument_variable_kinds[type_constructor],
                                        type_constructor_kinds);
                            } catch (const TypeError &e) {
                                throw TypeError(
                                        "Line " +
                                        std::to_string(program->data_constructors[data_constructor]->line) +
                                        " " + e.what());
                            }
                            try {
                                unify(k, std::make_shared<StarKind>());
                            } catch (const TypeError &e) {
                                throw TypeError(
                                        "Line " +
                                        std::to_string(program->data_constructors[data_constructor]->line) +
                                        " invalid type in data constructor.");
                            }
                            data_constructor_type = std::make_shared<TypeApplication>(
                                    std::make_shared<TypeApplication>(
                                            std::make_shared<TypeConstructor>("->"),
                                            program->data_constructors[data_constructor]->types[i]),
                                    data_constructor_type);
                        }
                        assumptions[data_constructor] = data_constructor_type;
                        data_constructor_arities[data_constructor] = program->data_constructors[data_constructor]->types.size();
                    }
                }
                for (const std::string &type_constructor: current) {
                    type_constructor_kinds[type_constructor] = generalise(type_constructor_kinds[type_constructor]);
                }
            }
        }
    }

    type_inference_declarations(
            assumptions,
            data_constructor_arities,
            program->bindings,
            program->type_signatures);
}
