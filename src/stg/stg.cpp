#include <string>
#include <map>
#include <memory>
#include "stg/stg.hpp"
#include "parser/syntax.hpp"

std::tuple<std::unique_ptr<STGLambdaForm>, std::unique_ptr<STGLet>, STGLet*> translate_string_literal(
        const std::string &s,
        unsigned long *next_variable_name) {
    if (s.empty()) {
        return std::make_tuple(
                std::make_unique<STGLambdaForm>(
                        std::set<std::string>(),
                        std::set<std::string>(),
                        false,
                        std::make_unique<STGConstructor>("[]")),
                nullptr,
                nullptr);
    }

    std::vector<std::string> args = {
            "." + std::to_string(*next_variable_name),
            "." + std::to_string((*next_variable_name)+1)};
    std::unique_ptr<STGLambdaForm> result = std::make_unique<STGLambdaForm>(
            std::set<std::string>(args.begin(), args.end()),
            std::set<std::string>(),
            false,
            std::make_unique<STGConstructor>(":", args));

    std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
    bindings["." + std::to_string((*next_variable_name)++)] = std::make_unique<STGLambdaForm>(
            std::set<std::string>(),
            std::set<std::string>(),
            false,
            std::make_unique<STGLiteral>(s[0]));
    std::unique_ptr<STGLet> definitions = std::make_unique<STGLet>(
            std::move(bindings),
            nullptr,
            false);
    STGLet* bottom_definition = definitions.get();


    for (int i = 1; i < s.length(); i++) {
        args = {
            "." + std::to_string((*next_variable_name)+1),
            "." + std::to_string((*next_variable_name)+2)};
        std::unique_ptr<STGExpression> cons = std::make_unique<STGConstructor>(
                ":",
                args);

        bindings = std::map<std::string, std::unique_ptr<STGLambdaForm>>();
        bindings["." + std::to_string((*next_variable_name)++)] = std::make_unique<STGLambdaForm>(
                std::set<std::string>(args.begin(), args.end()),
                std::set<std::string>(),
                false,
                std::move(cons));
        definitions = std::make_unique<STGLet>(
                std::move(bindings),
                std::move(definitions),
                false);

        bindings = std::map<std::string, std::unique_ptr<STGLambdaForm>>();
        bindings["." + std::to_string((*next_variable_name)++)] = std::make_unique<STGLambdaForm>(
                std::set<std::string>(),
                std::set<std::string>(),
                false,
                std::make_unique<STGLiteral>(s[i]));

        definitions = std::make_unique<STGLet>(
                std::move(bindings),
                std::move(definitions),
                false);
    }

    bindings = std::map<std::string, std::unique_ptr<STGLambdaForm>>();
    bindings["." + std::to_string((*next_variable_name)++)] = std::make_unique<STGLambdaForm>(
            std::set<std::string>(),
            std::set<std::string>(),
            false,
            std::make_unique<STGConstructor>("[]"));
    definitions = std::make_unique<STGLet>(
            std::move(bindings),
            std::move(definitions),
            false);

    return std::make_tuple(std::move(result), std::move(definitions), bottom_definition);
}

std::tuple<std::unique_ptr<STGLambdaForm>, std::unique_ptr<STGLet>, STGLet*> translate_variable(
        const std::unique_ptr<Expression> &expr,
        const std::map<std::string, std::string> &variable_renamings) {
    auto var = dynamic_cast<Variable*>(expr.get());
    std::unique_ptr<STGVariable> translated_var;
    if (variable_renamings.count(var->name) > 0) {
        translated_var = std::make_unique<STGVariable>(variable_renamings.at(var->name));
    } else {
        translated_var = std::make_unique<STGVariable>(var->name);
    }
    std::set<std::string> free_variables;
    free_variables.insert(translated_var->name);
    auto lambda_form = std::make_unique<STGLambdaForm>(
            free_variables,
            std::set<std::string>(),
            true,
            std::move(translated_var));
    return std::make_tuple(std::move(lambda_form), nullptr, nullptr);
}

std::tuple<std::unique_ptr<STGLambdaForm>, std::unique_ptr<STGLet>, STGLet*> translate_literal(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name) {
    auto lit = dynamic_cast<Literal*>(expr.get());
    std::variant<int, std::string, char> value = lit->value;
    if (std::holds_alternative<int>(value)) {
        return std::make_tuple(
                std::make_unique<STGLambdaForm>(
                        std::set<std::string>(),
                        std::set<std::string>(),
                        false,
                        std::make_unique<STGLiteral>(std::get<int>(value))),
                nullptr,
                nullptr);
    } else if (std::holds_alternative<std::string>(value)) {
        return translate_string_literal(std::get<std::string>(value), next_variable_name);
    } else if (std::holds_alternative<char>(value)) {
        return std::make_tuple(
                std::make_unique<STGLambdaForm>(
                        std::set<std::string>(),
                        std::set<std::string>(),
                        false,
                        std::make_unique<STGLiteral>(std::get<char>(value))),
                nullptr,
                nullptr);
    }
}

std::tuple<std::unique_ptr<STGLambdaForm>, std::unique_ptr<STGLet>, STGLet*> translate_abstraction(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        const std::map<std::string, std::string> &variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities) {
    auto e = &expr;
    std::set<std::string> argument_variables;
    while ((*e)->get_form() == expform::abstraction) {
        auto abstraction = dynamic_cast<Abstraction*>(e->get());
        argument_variables.insert(
                abstraction->args.begin(),
                abstraction->args.end());
        e = &(abstraction->body);
//
//        std::set<std::string> names_that_depend_on_arguments = argument_variables;
//
//        while ((*current)->get_form() == stgform::let) {
//            auto let = dynamic_cast<STGLet *>(current->get());
//
//            std::set<std::string> defined_names;
//            bool can_extract = true;
//            for (auto &[n, lambda_form]: let->bindings) {
//                defined_names.insert(n);
//                if (can_extract) {
//                    for (const auto &free_variable: lambda_form->free_variables) {
//                        if (names_that_depend_on_arguments.count(free_variable)) {
//                            can_extract = false;
//                            break;
//                        }
//                    }
//                }
//            }
//
//            if (can_extract) {
//                for (auto &[n, lambda_form]: let->bindings) {
//                    bindings[n] = std::move(lambda_form);
//                }
//                if (current == &translated) {
//                    translated = std::move(let->expr);
//                } else {
//                    (*parent)
//                }
//            } else {
//                names_that_depend_on_arguments.insert(defined_names.begin(), defined_names.end());
//                parent = current;
//                current = &let->expr;
//            }
//        }
    }
}

std::tuple<std::unique_ptr<STGLambdaForm>, std::unique_ptr<STGLet>, STGLet*> translate_expression(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        const std::map<std::string, std::string> &variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities) {
    switch(expr->get_form()) {
        case expform::variable:
            return translate_variable(expr, variable_renamings);
        case expform::literal:
            return translate_literal(expr, next_variable_name);
    }
}

std::unique_ptr<STGProgram> translate(const std::unique_ptr<Program> &program) {
    std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
    unsigned long next_variable_name = 0;

    for (const auto &[name, expr]: program->bindings) {
        auto translated = translate_expression(
                expr,
                &next_variable_name,
                std::map<std::string, std::string>(),
                program->data_constructor_arities);

        bindings[name] = std::move(std::get<0>(translated));

        std::unique_ptr<STGLet> definition = std::move(std::get<1>(translated));
        while (definition) {
            for (auto &[n, lambda_form]: definition->bindings) {
                bindings[n] = std::move(lambda_form);
            }
            definition = std::unique_ptr<STGLet>(dynamic_cast<STGLet*>(definition->expr.release()));
        }
    }

    return std::make_unique<STGProgram>(std::move(bindings));
}
