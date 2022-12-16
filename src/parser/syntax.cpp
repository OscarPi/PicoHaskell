#include <algorithm>
#include "parser/syntax.hpp"

ConstructorPattern::ConstructorPattern(
        const int &line, std::string name,
        const std::vector<Pattern *> &args): Pattern(line), name(std::move(name)) {
    for (const auto &arg: args) {
        this->args.emplace_back(arg);
    }
}

Case::Case(
        const int &line,
        Expression * const &exp,
        const std::vector<std::pair<Pattern*, Expression*>> &alts): Expression(line), exp(exp) {
    for (const auto &alt: alts) {
        this->alts.emplace_back(alt.first, alt.second);
    }
}

Let::Let(
        const int &line,
        const std::map<std::string, Expression*> &bindings,
        const std::map<std::string, type> &type_signatures,
        Expression * const &e): Expression(line), type_signatures(type_signatures), e(e) {
    for (auto const &[name, exp] : bindings) {
        this->bindings.emplace(name, exp);
    }
}

void Program::add_type_signatures(const std::vector<std::string> &names, const type &t) {
    for (const auto& name: names) {
        if (type_signatures.count(name) > 0) {
            throw ParseError("Multiple type signatures for the same name are not allowed.");
        }
        type_signatures[name] = t;
    }
}

void Program::add_type_constructor(
        int line,
        const std::string &name,
        const std::vector<std::string> &argument_variables,
        const std::vector<DConstructor*> &new_data_constructors) {

    std::vector<std::string> data_constructor_names;
    for (const auto& constructor: new_data_constructors) {
        constructor->type_constructor = name;
        if (data_constructors.count(constructor->name) > 0) {
            throw ParseError(
                    "Line " +
                    std::to_string(constructor->line) +
                    ": data constructor called " +
                    constructor->name +
                    " already exists."
            );
        }
        data_constructor_names.push_back(constructor->name);
        data_constructors.emplace(constructor->name, constructor);
    }

    auto type_constructor = std::make_unique<TConstructor>(line, name, argument_variables, data_constructor_names);
    if (type_constructors.count(type_constructor->name) > 0) {
        throw ParseError(
                "Line " +
                std::to_string(type_constructor->line) +
                ": type constructor called " +
                type_constructor->name +
                " already exists."
        );
    }
    type_constructors[type_constructor->name] = std::move(type_constructor);
}

void Program::add_variable(const int &line, const std::string &name, Expression * const &exp) {
    if (bindings.count(name) > 0) {
        throw ParseError(
                "Line " +
                std::to_string(line) +
                ": multiple bindings to the name " +
                name +
                "."
        );
    }
    bindings.emplace(name, exp);
}

void Program::add_named_function(const int &line, const std::string &name, const std::vector<std::string> &args,
                                 Expression * const &body) {
    if (bindings.count(name) > 0) {
        throw ParseError(
                "Line " +
                std::to_string(line) +
                ": multiple bindings to the name " +
                name +
                "."
        );
    }
    bindings[name] = std::make_unique<Lambda>(line, args, body);
}

Expression *make_if_expression(
        const int &line,
        Expression * const &e1,
        Expression * const &e2,
        Expression * const &e3) {
    const auto t = new ConstructorPattern(line, "True", std::vector<Pattern*>());
    const auto f = new ConstructorPattern(line, "False", std::vector<Pattern*>());
    const auto alt1 = std::make_pair(t, e2);
    const auto alt2 = std::make_pair(f, e3);
    const std::vector<std::pair<Pattern*, Expression*>> alts = {alt1, alt2};
    return new Case(line, e1, alts);
}

Expression *make_list_expression(const int &line, const std::vector<Expression *> &elements) {
    Expression *list = new Constructor(line, "[]");
    for (int i = elements.size() - 1; i >= 0; i--) {
        list = new Application(
                line,
                new Application(
                        line,
                        new Constructor(line, ":"),
                        elements[i]),
                list);
    }
    return list;
}

Expression *make_tuple_expression(const int &line, const std::vector<Expression*> &elements) {
    Expression *tuple = new Constructor(
            line,
            "(" + std::string(elements.size() - 1, ',') + ")");

    for (const auto &e: elements) {
        tuple = new Application(line, tuple, e);
    }
    return tuple;
}

Expression *make_let_expression(const int &line, const declist &decls, Expression * const &e) {
    std::map<std::string, Expression*> bindings;
    std::map<std::string, type> type_signatures;

    for (const auto &signature: std::get<0>(decls)) {
        for (const auto &name: signature.first) {
            if (type_signatures.count(name) > 0) {
                throw ParseError(
                        "Line " +
                        std::to_string(line) +
                        ": multiple type signatures for the same name are not allowed."
                );
            }
            type_signatures[name] = signature.second;
        }
    }

    for (const auto &function: std::get<1>(decls)) {
        if (bindings.count(std::get<0>(function)) > 0) {
            throw ParseError(
                    "Line " +
                    std::to_string(line) +
                    ": multiple bindings to the name " +
                    std::get<0>(function) +
                    "."
            );
        }
        bindings[std::get<0>(function)] = new Lambda(line, std::get<1>(function), std::get<2>(function));
    }

    for (const auto &variable: std::get<2>(decls)) {
        if (bindings.count(variable.first) > 0) {
            throw ParseError(
                    "Line " +
                    std::to_string(line) +
                    ": multiple bindings to the name " +
                    variable.first +
                    "."
            );
        }
        bindings[variable.first] = variable.second;
    }

    return new Let(line, bindings, type_signatures, e);
}

Pattern *make_list_pattern(const int &line, const std::vector<Pattern*> &elements) {
    Pattern *list = new ConstructorPattern(line, "[]", {});
    for (int i = elements.size() - 1; i >= 0; i--) {
        list = new ConstructorPattern(
                line,
                ":",
                {elements[i], list});
    }
    return list;
}

Pattern *make_tuple_pattern(const int &line, const std::vector<Pattern*> &elements) {
    return new ConstructorPattern(line, "(" + std::string(elements.size() - 1, ',') + ")", elements);
}
