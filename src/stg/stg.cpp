#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include "stg/stg.hpp"
#include "parser/syntax.hpp"

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_expression(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        const std::map<std::string, std::string> &variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities);

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_string_literal(
        const std::string &s,
        unsigned long *next_variable_name) {
    if (s.empty()) {
        return std::make_pair(
                std::make_unique<STGLambdaForm>(
                        std::set<std::string>(),
                        std::vector<std::string>(),
                        false,
                        std::make_unique<STGConstructor>("[]")),
                std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>());
    }

    std::vector<std::string> args = {
            "." + std::to_string(*next_variable_name),
            "." + std::to_string((*next_variable_name)+1)};
    std::unique_ptr<STGLambdaForm> result = std::make_unique<STGLambdaForm>(
            std::set<std::string>(args.begin(), args.end()),
            std::vector<std::string>(),
            false,
            std::make_unique<STGConstructor>(":", args));

    std::map<std::string, std::unique_ptr<STGLambdaForm>> definition;
    definition["." + std::to_string((*next_variable_name)++)] = std::make_unique<STGLambdaForm>(
            std::set<std::string>(),
            std::vector<std::string>(),
            false,
            std::make_unique<STGLiteral>(s[0]));

    std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> definitions;
    definitions.push_back(std::move(definition));

    for (int i = 1; i < s.length(); i++) {
        args = {
            "." + std::to_string((*next_variable_name)+1),
            "." + std::to_string((*next_variable_name)+2)};
        std::unique_ptr<STGExpression> cons = std::make_unique<STGConstructor>(
                ":",
                args);

        definition = std::map<std::string, std::unique_ptr<STGLambdaForm>>();
        definition["." + std::to_string((*next_variable_name)++)] = std::make_unique<STGLambdaForm>(
                std::set<std::string>(args.begin(), args.end()),
                std::vector<std::string>(),
                false,
                std::move(cons));
        definitions.push_back(std::move(definition));

        definition = std::map<std::string, std::unique_ptr<STGLambdaForm>>();
        definition["." + std::to_string((*next_variable_name)++)] = std::make_unique<STGLambdaForm>(
                std::set<std::string>(),
                std::vector<std::string>(),
                false,
                std::make_unique<STGLiteral>(s[i]));

        definitions.push_back(std::move(definition));
    }

    definition = std::map<std::string, std::unique_ptr<STGLambdaForm>>();
    definition["." + std::to_string((*next_variable_name)++)] = std::make_unique<STGLambdaForm>(
            std::set<std::string>(),
            std::vector<std::string>(),
            false,
            std::make_unique<STGConstructor>("[]"));
    definitions.push_back(std::move(definition));

    return std::make_pair(std::move(result), std::move(definitions));
}

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_variable(
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
            std::vector<std::string>(),
            true,
            std::move(translated_var));
    return std::make_pair(
            std::move(lambda_form),
            std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>());
}

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_literal(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name) {
    auto lit = dynamic_cast<Literal*>(expr.get());
    std::variant<int, std::string, char> value = lit->value;
    if (std::holds_alternative<int>(value)) {
        return std::make_pair(
                std::make_unique<STGLambdaForm>(
                        std::set<std::string>(),
                        std::vector<std::string>(),
                        false,
                        std::make_unique<STGLiteral>(std::get<int>(value))),
                std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>());
    } else if (std::holds_alternative<std::string>(value)) {
        return translate_string_literal(std::get<std::string>(value), next_variable_name);
    } else if (std::holds_alternative<char>(value)) {
        return std::make_pair(
                std::make_unique<STGLambdaForm>(
                        std::set<std::string>(),
                        std::vector<std::string>(),
                        false,
                        std::make_unique<STGLiteral>(std::get<char>(value))),
                std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>());
    }
}

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_constructor(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        const std::map<std::string, std::string> &variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities) {

}

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_abstraction(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        std::map<std::string, std::string> variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities) {
    auto expression = &expr;
    std::vector<std::string> argument_variables;
    do {
        auto abstraction = dynamic_cast<Abstraction*>(expression->get());
        for (const auto &arg: abstraction->args) {
            std::string new_name = "." + std::to_string((*next_variable_name)++);
            argument_variables.push_back(new_name);
            variable_renamings[arg] = new_name;
        }
        expression = &(abstraction->body);
    } while ((*expression)->get_form() == expform::abstraction);

    auto translated = translate_expression(
            *expression,
            next_variable_name,
            variable_renamings,
            data_constructor_arities);

    std::set<std::string> free_variables = translated.first->free_variables;
    std::set<std::string> names_that_depend_on_arguments;
    for (const auto &v: argument_variables) {
        free_variables.erase(v);
        names_that_depend_on_arguments.insert(v);
    }
    argument_variables.insert(
            argument_variables.end(),
            translated.first->argument_variables.begin(),
            translated.first->argument_variables.end());
    std::unique_ptr<STGExpression> body_expression = std::move(translated.first->expr);
    std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> definitions;
    for (auto &definition: translated.second) {
        std::set<std::string> defined_names;
        std::set<std::string> free_variables_in_definition;
        bool depends_on_arguments = false;
        for (auto &[name, lambda_form]: definition) {
            defined_names.insert(name);
            for (const auto &free_variable: lambda_form->free_variables) {
                free_variables_in_definition.insert(free_variable);
                if (names_that_depend_on_arguments.count(free_variable)) {
                    depends_on_arguments = true;
                }
            }
        }

        if (!depends_on_arguments) {
            definitions.push_back(std::move(definition));
        } else {
            bool recursive = false;
            for (const auto &name: defined_names) {
                free_variables.erase(name);
                names_that_depend_on_arguments.insert(name);
            }
            for (const auto &name: free_variables_in_definition) {
                if (defined_names.count(name)) {
                    recursive = true;
                } else if (!std::count(argument_variables.begin(), argument_variables.end(), name)) {
                    free_variables.insert(name);
                }
            }
            body_expression = std::make_unique<STGLet>(
                    std::move(definition),
                    std::move(body_expression),
                    recursive);
        }
    }

    return std::make_pair(
            std::make_unique<STGLambdaForm>(
                    free_variables,
                    argument_variables,
                    false,
                    std::move(body_expression)),
            std::move(definitions));
}

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_expression(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        const std::map<std::string, std::string> &variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities) {
    switch(expr->get_form()) {
        case expform::variable:
            return translate_variable(expr, variable_renamings);
        case expform::literal:
            return translate_literal(expr, next_variable_name);
        case expform::abstraction:
            return translate_abstraction(expr, next_variable_name, variable_renamings, data_constructor_arities);
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

        bindings[name] = std::move(translated.first);

        auto definitions = std::move(translated.second);
        for (auto &definition: definitions) {
            for (auto &[n, lambda_form]: definition) {
                bindings[n] = std::move(lambda_form);
            }
        }
    }

    return std::make_unique<STGProgram>(std::move(bindings));
}
