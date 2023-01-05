#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include "stg/stg.hpp"
#include "parser/syntax.hpp"
#include "types/type_check.hpp"

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_expression(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        const std::map<std::string, std::string> &variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities);

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
        const std::map<std::string, size_t> &data_constructor_arities) {
    auto constructor = dynamic_cast<Constructor*>(expr.get());

    if (data_constructor_arities.at(constructor->name) == 0) {
        return std::make_pair(
                std::make_unique<STGLambdaForm>(
                        std::set<std::string>(),
                        std::vector<std::string>(),
                        false,
                        std::make_unique<STGConstructor>(constructor->name)),
                std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>());
    } else {
        std::vector<std::string> argument_variables;
        for (int i = 0; i < data_constructor_arities.at(constructor->name); i++) {
            argument_variables.push_back("." + std::to_string((*next_variable_name)++));
        }
        std::string var_name = "." + std::to_string((*next_variable_name)++);
        std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
        bindings[var_name] = std::make_unique<STGLambdaForm>(
                std::set<std::string>(argument_variables.begin(), argument_variables.end()),
                std::vector<std::string>(),
                false,
                std::make_unique<STGConstructor>(constructor->name, argument_variables));
        return std::make_pair(
                std::make_unique<STGLambdaForm>(
                        std::set<std::string>(),
                        argument_variables,
                        false,
                        std::make_unique<STGLet>(
                                std::move(bindings),
                                std::make_unique<STGVariable>(var_name),
                                false)),
                std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>());
    }
}

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_built_in_op(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        const std::map<std::string, std::string> &variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities) {
    std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> definitions;

    auto op = dynamic_cast<BuiltInOp*>(expr.get());

    std::string left;
    if (op->left) {
        auto translated = translate_expression(
                op->left,
                next_variable_name,
                variable_renamings,
                data_constructor_arities);

        for (auto &definition: translated.second) {
            definitions.push_back(std::move(definition));
        }

        if (translated.first->expr->get_form() == stgform::variable) {
            left = dynamic_cast<STGVariable*>(translated.first->expr.get())->name;
        } else {
            std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
            std::string name = "." + std::to_string((*next_variable_name)++);
            bindings[name] = std::move(translated.first);
            definitions.push_back(std::move(bindings));
            left = name;
        }
    }

    std::string right;
    auto translated = translate_expression(
            op->right,
            next_variable_name,
            variable_renamings,
            data_constructor_arities);

    for (auto &definition: translated.second) {
        definitions.push_back(std::move(definition));
    }

    if (translated.first->expr->get_form() == stgform::variable) {
        right = dynamic_cast<STGVariable *>(translated.first->expr.get())->name;
    } else {
        std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
        std::string name = "." + std::to_string((*next_variable_name)++);
        bindings[name] = std::move(translated.first);
        definitions.push_back(std::move(bindings));
        right = name;
    }

    std::set<std::string> free_variables;
    if (!left.empty()) {
        free_variables.insert(left);
    }
    free_variables.insert(right);
    return std::make_pair(
            std::make_unique<STGLambdaForm>(
                    free_variables,
                    std::vector<std::string>(),
                    true,
                    std::make_unique<STGPrimitiveOp>(left, right, op->op)),
            std::move(definitions));
}

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_application(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        const std::map<std::string, std::string> &variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities) {
    std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> definitions;
    std::vector<std::string> argument_variables;

    auto expression = &expr;
    do {
        auto app = dynamic_cast<Application*>(expression->get());

        auto translated = translate_expression(
                app->right,
                next_variable_name,
                variable_renamings,
                data_constructor_arities);

        for (auto &definition: translated.second) {
            definitions.push_back(std::move(definition));
        }

        if (translated.first->expr->get_form() == stgform::variable) {
            argument_variables.insert(
                    argument_variables.begin(),
                    dynamic_cast<STGVariable*>(translated.first->expr.get())->name);
        } else {
            std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
            std::string name = "." + std::to_string((*next_variable_name)++);
            bindings[name] = std::move(translated.first);
            definitions.push_back(std::move(bindings));
            argument_variables.insert(argument_variables.begin(), name);
        }

        expression = &(app->left);
    } while ((*expression)->get_form() == expform::application);

    if ((*expression)->get_form() == expform::constructor) {
        std::string constructor_name = dynamic_cast<Constructor*>(expression->get())->name;
        if (argument_variables.size() < data_constructor_arities.at(constructor_name)) {
            std::vector<std::string> additional_argument_variables;
            for (int i = argument_variables.size(); i < data_constructor_arities.at(constructor_name); i++) {
                additional_argument_variables.push_back("." + std::to_string((*next_variable_name)++));
            }
            std::vector<std::string> combined_argument_variables = argument_variables;
            combined_argument_variables.insert(
                    combined_argument_variables.end(),
                    additional_argument_variables.begin(),
                    additional_argument_variables.end());
            std::string var_name = "." + std::to_string((*next_variable_name)++);
            std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
            bindings[var_name] = std::make_unique<STGLambdaForm>(
                    std::set<std::string>(combined_argument_variables.begin(), combined_argument_variables.end()),
                    std::vector<std::string>(),
                    false,
                    std::make_unique<STGConstructor>(constructor_name, combined_argument_variables));
            return std::make_pair(
                    std::make_unique<STGLambdaForm>(
                            std::set<std::string>(argument_variables.begin(), argument_variables.end()),
                            additional_argument_variables,
                            false,
                            std::make_unique<STGLet>(
                                    std::move(bindings),
                                    std::make_unique<STGVariable>(var_name),
                                    false)),
                    std::move(definitions));
        } else {
            return std::make_pair(
                    std::make_unique<STGLambdaForm>(
                            std::set<std::string>(argument_variables.begin(), argument_variables.end()),
                            std::vector<std::string>(),
                            false,
                            std::make_unique<STGConstructor>(constructor_name, argument_variables)),
                    std::move(definitions));
        }
    }

    auto translated = translate_expression(
            *expression,
            next_variable_name,
            variable_renamings,
            data_constructor_arities);

    for (auto &definition: translated.second) {
        definitions.push_back(std::move(definition));
    }

    if (translated.first->expr->get_form() == stgform::variable) {
        std::string name = dynamic_cast<STGVariable*>(translated.first->expr.get())->name;
        std::set<std::string> free_variables;
        free_variables.insert(name);
        free_variables.insert(argument_variables.begin(), argument_variables.end());
        return std::make_pair(
                std::make_unique<STGLambdaForm>(
                        free_variables,
                        std::vector<std::string>(),
                        true,
                        std::make_unique<STGApplication>(name, argument_variables)),
                std::move(definitions));
    } else {
        std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
        std::string name = "." + std::to_string((*next_variable_name)++);
        bindings[name] = std::move(translated.first);
        definitions.push_back(std::move(bindings));
        std::set<std::string> free_variables;
        free_variables.insert(name);
        free_variables.insert(argument_variables.begin(), argument_variables.end());
        return std::make_pair(
                std::make_unique<STGLambdaForm>(
                        free_variables,
                        std::vector<std::string>(),
                        true,
                        std::make_unique<STGApplication>(name, argument_variables)),
                std::move(definitions));
    }

}

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_let(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        std::map<std::string, std::string> variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities) {
    auto let = dynamic_cast<Let*>(expr.get());

    std::vector<std::string> names_defined;
    for (const auto &[name, _]: let->bindings) {
        variable_renamings[name] = "." + std::to_string((*next_variable_name)++);
        names_defined.push_back(name);
    }
    std::map<std::string, std::set<std::string>> dependencies;
    for (const auto &[name, expression]: let->bindings) {
        dependencies[name] = std::set<std::string>();
        for (const auto &free_variable: find_free_variables(expression)) {
            if (std::count(names_defined.begin(), names_defined.end(), free_variable)) {
                dependencies[name].insert(free_variable);
            }
        }
    }
    auto dependency_groups = dependency_analysis(names_defined, dependencies);

    std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> definitions;

    for (const auto &group: dependency_groups) {
        std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
        std::set<std::string> names_defined_in_group;
        for (const auto &name: group) {
            names_defined_in_group.insert(variable_renamings[name]);
        }
        for (const auto &name: group) {
            auto translated = translate_expression(
                    let->bindings.at(name),
                    next_variable_name,
                    variable_renamings,
                    data_constructor_arities);
            for (auto &definition: translated.second) {
                bool depends_on_names_in_group = false;
                for (auto &[n, lambda_form]: definition) {
                    for (const auto &free_variable: lambda_form->free_variables) {
                        if (names_defined_in_group.count(free_variable)) {
                            depends_on_names_in_group = true;
                            break;
                        }
                    }
                }

                if (!depends_on_names_in_group) {
                    definitions.push_back(std::move(definition));
                } else {
                    for (auto &[n, lambda_form]: definition) {
                        names_defined_in_group.insert(n);
                        bindings[n] = std::move(lambda_form);
                    }
                }
            }
            bindings[variable_renamings[name]] = std::move(translated.first);
        }
        definitions.push_back(std::move(bindings));
    }

    auto translated = translate_expression(
            let->e,
            next_variable_name,
            variable_renamings,
            data_constructor_arities);

   for (auto &definition: translated.second) {
       definitions.push_back(std::move(definition));
   }

   return std::make_pair(std::move(translated.first), std::move(definitions));
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
    std::vector<std::pair<std::map<std::string, std::unique_ptr<STGLambdaForm>>, bool>> definitions_that_depend_on_arguments;
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
                } else if (!names_that_depend_on_arguments.count(name)) {
                    free_variables.insert(name);
                }
            }
            definitions_that_depend_on_arguments.emplace_back(std::move(definition), recursive);
        }
    }

    for (auto it = definitions_that_depend_on_arguments.rbegin();
            it != definitions_that_depend_on_arguments.rend(); ++it) {
        auto &[definition, recursive] = *it;
        body_expression = std::make_unique<STGLet>(
                std::move(definition),
                std::move(body_expression),
                recursive);
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
        case expform::let:
            return translate_let(expr, next_variable_name, variable_renamings, data_constructor_arities);
        case expform::constructor:
            return translate_constructor(expr, next_variable_name, data_constructor_arities);
        case expform::application:
            return translate_application(expr, next_variable_name, variable_renamings, data_constructor_arities);
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
