#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <list>
#include "stg/stg.hpp"
#include "parser/syntax.hpp"
#include "types/type_check.hpp"

void add_definition(
        const std::string &name,
        std::unique_ptr<STGLambdaForm> &&lambda_form,
        std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> &definitions) {
    std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
    bindings[name] = std::move(lambda_form);
    definitions.push_back(std::move(bindings));
}

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
    std::variant<int, char> value = dynamic_cast<Literal*>(expr.get())->value;
    return std::make_pair(
            std::make_unique<STGLambdaForm>(
                    std::set<std::string>(),
                    std::vector<std::string>(),
                    false,
                    std::make_unique<STGLiteral>(value)),
            std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>());
}

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_constructor(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        const std::map<std::string, size_t> &data_constructor_arities) {
    auto constructor = dynamic_cast<Constructor*>(expr.get());

    std::vector<std::string> argument_variables;
    for (int i = 0; i < data_constructor_arities.at(constructor->name); i++) {
        argument_variables.push_back("." + std::to_string((*next_variable_name)++));
    }
    return std::make_pair(
            std::make_unique<STGLambdaForm>(
                    std::set<std::string>(),
                    argument_variables,
                    false,
                    std::make_unique<STGConstructor>(
                            constructor->name,
                            argument_variables)),
            std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>());
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
            std::string name = "." + std::to_string((*next_variable_name)++);
            add_definition(name, std::move(translated.first), definitions);
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
        std::string name = "." + std::to_string((*next_variable_name)++);
        add_definition(name, std::move(translated.first), definitions);
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

std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> capture_definitions_that_depend_on_names(
        std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> &&definitions,
        std::unique_ptr<STGExpression> &expr,
        std::set<std::string> &free_variables_in_expr,
        const std::vector<std::string> &names) {
    std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> independent_definitions;
    std::vector<std::pair<std::map<std::string, std::unique_ptr<STGLambdaForm>>, bool>> definitions_that_depend_on_names;
    std::set<std::string> names_that_depend_on_names(names.begin(), names.end());
    for (auto &definition: definitions) {
        std::set<std::string> defined_names;
        std::set<std::string> free_variables_in_definition;
        bool depends_on_names = false;
        for (auto &[name, lambda_form]: definition) {
            defined_names.insert(name);
            for (const auto &free_variable: lambda_form->free_variables) {
                free_variables_in_definition.insert(free_variable);
                if (names_that_depend_on_names.count(free_variable)) {
                    depends_on_names = true;
                }
            }
        }

        if (!depends_on_names) {
            independent_definitions.push_back(std::move(definition));
        } else {
            bool recursive = false;
            for (const auto &name: defined_names) {
                free_variables_in_expr.erase(name);
                names_that_depend_on_names.insert(name);
            }
            for (const auto &name: free_variables_in_definition) {
                if (defined_names.count(name)) {
                    recursive = true;
                } else if (!names_that_depend_on_names.count(name)) {
                    free_variables_in_expr.insert(name);
                }
            }
            definitions_that_depend_on_names.emplace_back(std::move(definition), recursive);
        }
    }

    for (auto it = definitions_that_depend_on_names.rbegin();
         it != definitions_that_depend_on_names.rend(); ++it) {
        auto &[definition, recursive] = *it;
        expr = std::make_unique<STGLet>(
                std::move(definition),
                std::move(expr),
                recursive);
    }

    return independent_definitions;
}

std::unique_ptr<STGExpression> translate_alt_expression(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        const std::map<std::string, std::string> &variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities,
        const std::vector<std::string> &names_bound_in_pattern,
        std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> &definitions,
        std::set<std::string> &free_variables) {
    auto alt_expr_translated = translate_expression(
            expr,
            next_variable_name,
            variable_renamings,
            data_constructor_arities);
    auto alt_definitions = std::move(alt_expr_translated.second);
    std::set<std::string> free_variables_in_alt;
    std::unique_ptr<STGExpression> alt_expr;
    if (!alt_expr_translated.first->argument_variables.empty()) {
        std::string name = "." + std::to_string((*next_variable_name)++);
        add_definition(name, std::move(alt_expr_translated.first),  alt_definitions);
        free_variables_in_alt.insert(name);
        alt_expr = std::make_unique<STGVariable>(name);
    } else {
        free_variables_in_alt = alt_expr_translated.first->free_variables;
        alt_expr = std::move(alt_expr_translated.first->expr);
    }

    for (const auto &variable: names_bound_in_pattern) {
        free_variables_in_alt.erase(variable);
    }

    auto independent_definitions = capture_definitions_that_depend_on_names(
            std::move(alt_definitions),
            alt_expr,
            free_variables_in_alt,
            names_bound_in_pattern);

    for (auto &definition: independent_definitions) {
        definitions.push_back(std::move(definition));
    }

    free_variables.insert(free_variables_in_alt.begin(), free_variables_in_alt.end());

    return alt_expr;
}

std::unique_ptr<STGExpression> copy(const std::unique_ptr<STGExpression> &expr);

std::unique_ptr<STGLambdaForm> copy(const std::unique_ptr<STGLambdaForm> &lambda_form) {
    return std::make_unique<STGLambdaForm>(
            lambda_form->free_variables,
            lambda_form->argument_variables,
            lambda_form->updatable,
            copy(lambda_form->expr));
}

std::unique_ptr<STGExpression> copy(const std::unique_ptr<STGExpression> &expr) {
    switch(expr->get_form()) {
        case stgform::variable:
            return std::make_unique<STGVariable>(dynamic_cast<STGVariable*>(expr.get())->name);
        case stgform::constructor:
            return std::make_unique<STGConstructor>(
                    dynamic_cast<STGConstructor*>(expr.get())->constructor_name,
                    dynamic_cast<STGConstructor*>(expr.get())->arguments);
        case stgform::literal:
            return std::make_unique<STGLiteral>(dynamic_cast<STGLiteral*>(expr.get())->value);
        case stgform::literalcase: {
            auto cAsE = dynamic_cast<STGLiteralCase*>(expr.get());
            auto default_expr = copy(cAsE->default_expr);
            auto default_var = cAsE->default_var;
            auto expression = copy(cAsE->expr);
            std::vector<std::pair<STGLiteral, std::unique_ptr<STGExpression>>> alts;
            for (const auto &[lit, e]: cAsE->alts) {
                alts.emplace_back(lit, copy(e));
            }
            return std::make_unique<STGLiteralCase>(
                    std::move(expression),
                    std::move(alts),
                    default_var,
                    std::move(default_expr));
        }
        case stgform::algebraiccase: {
            auto cAsE = dynamic_cast<STGAlgebraicCase*>(expr.get());
            auto default_expr = copy(cAsE->default_expr);
            auto default_var = cAsE->default_var;
            auto expression = copy(cAsE->expr);
            std::vector<std::pair<STGPattern, std::unique_ptr<STGExpression>>> alts;
            for (const auto &[pat, e]: cAsE->alts) {
                alts.emplace_back(pat, copy(e));
            }
            return std::make_unique<STGAlgebraicCase>(
                    std::move(expression),
                    std::move(alts),
                    default_var,
                    std::move(default_expr));
        }
        case stgform::application:
            return std::make_unique<STGApplication>(
                    dynamic_cast<STGApplication*>(expr.get())->lhs,
                    dynamic_cast<STGApplication*>(expr.get())->arguments);
        case stgform::let: {
            auto let = dynamic_cast<STGLet*>(expr.get());
            std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
            for (const auto &[name, lambda_form]: let->bindings) {
                bindings[name] = copy(lambda_form);
            }
            return std::make_unique<STGLet>(
                    std::move(bindings),
                    copy(let->expr),
                    let->recursive);
        }
        case stgform::primitiveop:
            return std::make_unique<STGPrimitiveOp>(
                    dynamic_cast<STGPrimitiveOp*>(expr.get())->left,
                    dynamic_cast<STGPrimitiveOp*>(expr.get())->right,
                    dynamic_cast<STGPrimitiveOp*>(expr.get())->op);
    }
}

std::unique_ptr<STGExpression> translate_case(
        const std::vector<std::string> &variables,
        const std::list<std::tuple<std::list<Pattern*>, std::map<std::string, std::string>, const std::unique_ptr<Expression>*>> &alternatives,
        const std::vector<std::string> &names_bound_in_pattern,
        std::unique_ptr<STGExpression> &&default_expr,
        unsigned long *next_variable_name,
        const std::map<std::string, size_t> &data_constructor_arities,
        std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> &definitions,
        std::set<std::string> &free_variables) {
    if (variables.empty()) {
        const auto &[_, variable_renamings, expr] = *alternatives.begin();
        return translate_alt_expression(
                *expr,
                next_variable_name,
                variable_renamings,
                data_constructor_arities,
                names_bound_in_pattern,
                definitions,
                free_variables);
    }

    std::unique_ptr<STGExpression> next_group = std::move(default_expr);
    std::list<std::tuple<
            std::list<Pattern*>,
            std::map<std::string, std::string>,
            const std::unique_ptr<Expression>*>> variable_alts;
    std::map<std::string, std::list<std::tuple<
            std::list<Pattern*>,
            std::map<std::string, std::string>,
            const std::unique_ptr<Expression>*>>> constructor_alts;
    std::map<std::variant<int, char>, std::list<std::tuple<
            std::list<Pattern*>,
            std::map<std::string, std::string>,
            const std::unique_ptr<Expression>*>>> literal_alts;

    for (auto it = alternatives.rbegin(); ; ++it) {
        std::list<Pattern*> patterns;
        std::map<std::string, std::string> variable_renamings;
        const std::unique_ptr<Expression>* expr;
        patternform form;

        if (it != alternatives.rend()) {
            patterns = std::get<0>(*it);
            variable_renamings = std::get<1>(*it);
            expr = std::get<2>(*it);
            form = (*patterns.begin())->get_form();
            for (const auto &as: (*patterns.begin())->as) {
                variable_renamings[as] = variables[0];
            }
        }

        if (!variable_alts.empty() && (it == alternatives.rend() || (form != patternform::variable && form != patternform::wild))) {
            std::vector<std::string> new_variables;
            for (auto vit = variables.begin()+1; vit != variables.end(); vit++) {
                new_variables.push_back(*vit);
            }
            next_group = translate_case(
                    new_variables,
                    variable_alts,
                    names_bound_in_pattern,
                    std::move(next_group),
                    next_variable_name,
                    data_constructor_arities,
                    definitions,
                    free_variables);
            variable_alts.clear();
        } else if (!constructor_alts.empty() && (it == alternatives.rend() || form != patternform::constructor)) {
            std::vector<std::pair<STGPattern, std::unique_ptr<STGExpression>>> translated_alts;
            for (const auto &[constructor_name, alts]: constructor_alts) {
                std::vector<std::string> argument_variables;
                for (int i = 0; i < data_constructor_arities.at(constructor_name); i++) {
                    argument_variables.push_back("." + std::to_string((*next_variable_name)++));
                }

                std::vector<std::string> bound_names = names_bound_in_pattern;
                bound_names.insert(
                        bound_names.end(),
                        argument_variables.begin(),
                        argument_variables.end());
                std::vector<std::string> new_variables = argument_variables;
                for (auto vit = variables.begin()+1; vit != variables.end(); vit++) {
                    new_variables.push_back(*vit);
                }
                auto alt_expr = translate_case(
                        new_variables,
                        alts,
                        bound_names,
                        copy(next_group),
                        next_variable_name,
                        data_constructor_arities,
                        definitions,
                        free_variables);

                translated_alts.emplace_back(
                        STGPattern(constructor_name, argument_variables),
                        std::move(alt_expr));
            }

            next_group = std::make_unique<STGAlgebraicCase>(
                    std::make_unique<STGVariable>(variables[0]),
                    std::move(translated_alts),
                    "",
                    std::move(next_group));

            constructor_alts.clear();
        } else if (!literal_alts.empty() && (it == alternatives.rend() || form != patternform::literal)) {
            std::vector<std::pair<STGLiteral, std::unique_ptr<STGExpression>>> translated_alts;
            for (const auto &[literal_value, alts]: literal_alts) {
                std::vector<std::string> new_variables;
                for (auto vit = variables.begin()+1; vit != variables.end(); vit++) {
                    new_variables.push_back(*vit);
                }
                auto alt_expr = translate_case(
                        new_variables,
                        alts,
                        names_bound_in_pattern,
                        copy(next_group),
                        next_variable_name,
                        data_constructor_arities,
                        definitions,
                        free_variables);

                translated_alts.emplace_back(
                        STGLiteral(literal_value),
                        std::move(alt_expr));
            }

            next_group = std::make_unique<STGLiteralCase>(
                    std::make_unique<STGVariable>(variables[0]),
                    std::move(translated_alts),
                    "",
                    std::move(next_group));

            literal_alts.clear();
        }

        if (it == alternatives.rend()) {
            break;
        }

        if (form == patternform::variable || form == patternform::wild) {
            if (form == patternform::variable) {
                std::string name = dynamic_cast<VariablePattern *>(*patterns.begin())->name;
                variable_renamings[name] = variables[0];
            }
            patterns.pop_front();
            variable_alts.emplace_front(patterns, variable_renamings, expr);
        } else if (form == patternform::constructor) {
            std::string constructor_name = dynamic_cast<ConstructorPattern*>(*patterns.begin())->name;
            if (constructor_alts.count(constructor_name) == 0) {
                constructor_alts[constructor_name] = std::list<std::tuple<
                        std::list<Pattern*>,
                        std::map<std::string, std::string>,
                        const std::unique_ptr<Expression>*>>();
            }
            std::list<Pattern*> sub_patterns;
            for (const auto &sub_pattern: dynamic_cast<ConstructorPattern*>(*patterns.begin())->args) {
                sub_patterns.push_back(sub_pattern.get());
            }
            patterns.pop_front();
            patterns.splice(patterns.begin(), sub_patterns);
            constructor_alts[constructor_name].emplace_front(patterns, variable_renamings, expr);
        } else if (form == patternform::literal) {
            auto literal_value = dynamic_cast<LiteralPattern*>(*patterns.begin())->value;
            if (literal_alts.count(literal_value) == 0) {
                literal_alts[literal_value] = std::list<std::tuple<
                        std::list<Pattern*>,
                        std::map<std::string, std::string>,
                        const std::unique_ptr<Expression>*>>();
            }
            patterns.pop_front();
            literal_alts[literal_value].emplace_front(patterns, variable_renamings, expr);
        }
    }
    return std::move(next_group);
}

std::pair<std::unique_ptr<STGLambdaForm>, std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>>> translate_case(
        const std::unique_ptr<Expression> &expr,
        unsigned long *next_variable_name,
        std::map<std::string, std::string> variable_renamings,
        const std::map<std::string, size_t> &data_constructor_arities) {
    auto cAsE = dynamic_cast<Case*>(expr.get());

    std::string as;

    auto translated = translate_expression(
            cAsE->exp,
            next_variable_name,
            variable_renamings,
            data_constructor_arities);

    std::vector<std::map<std::string, std::unique_ptr<STGLambdaForm>>> definitions = std::move(translated.second);

    patternform first_alt_pattern_form = cAsE->alts[0].first->get_form();

    if (first_alt_pattern_form == patternform::wild || first_alt_pattern_form == patternform::variable) {
        if (!cAsE->alts[0].first->as.empty() || first_alt_pattern_form == patternform::variable) {
            as = "." + std::to_string((*next_variable_name)++);
            add_definition(as, std::move(translated.first), definitions);
            if (first_alt_pattern_form == patternform::variable) {
                variable_renamings[dynamic_cast<VariablePattern*>(cAsE->alts[0].first.get())->name] = as;
            }
            for (const std::string& name: cAsE->alts[0].first->as) {
                variable_renamings[name] = as;
            }
            auto alt_expr_translated = translate_expression(
                    cAsE->alts[0].second,
                    next_variable_name,
                    variable_renamings,
                    data_constructor_arities);
            for (auto &definition: alt_expr_translated.second) {
                definitions.push_back(std::move(definition));
            }
            return std::make_pair(std::move(alt_expr_translated.first), std::move(definitions));
        } else {
            return translate_expression(
                    cAsE->alts[0].second,
                    next_variable_name,
                    variable_renamings,
                    data_constructor_arities);
        }
    } else if (first_alt_pattern_form == patternform::literal || first_alt_pattern_form == patternform::constructor) {
        std::string default_var;
        std::unique_ptr<STGExpression> default_expr = std::make_unique<STGVariable>("case_error");
        std::set<std::string> free_variables;

        std::vector<std::pair<STGLiteral, std::unique_ptr<STGExpression>>> literal_alts;
        std::map<
                std::string,
                std::list<std::tuple<
                        std::list<Pattern*>,
                        std::map<std::string, std::string>,
                        const std::unique_ptr<Expression>*>>> constructor_alts;

        for (const auto &alt: cAsE->alts) {
            std::map<std::string, std::string> local_variable_renamings = variable_renamings;
            if (!alt.first->as.empty()) {
                if (as.empty()) {
                    as = "." + std::to_string((*next_variable_name)++);
                    add_definition(as, std::move(translated.first), definitions);
                }
                for (const auto &name: alt.first->as) {
                    local_variable_renamings[name] = as;
                }
            }

            if (alt.first->get_form() == patternform::constructor) {
                std::string constructor_name = dynamic_cast<ConstructorPattern*>(alt.first.get())->name;
                if (constructor_alts.count(constructor_name) == 0) {
                    constructor_alts[constructor_name] = std::list<std::tuple<
                            std::list<Pattern*>,
                            std::map<std::string, std::string>,
                            const std::unique_ptr<Expression>*>>();
                }
                std::list<Pattern*> sub_patterns;
                for (const auto &sub_pattern: dynamic_cast<ConstructorPattern*>(alt.first.get())->args) {
                    sub_patterns.push_back(sub_pattern.get());
                }
                constructor_alts[constructor_name].emplace_back(
                        sub_patterns,
                        local_variable_renamings,
                        &alt.second);
            } else {
                std::vector<std::string> names_bound_in_pattern;
                if (alt.first->get_form() == patternform::variable) {
                    std::string name = dynamic_cast<VariablePattern *>(alt.first.get())->name;
                    if (first_alt_pattern_form == patternform::constructor) {
                        if (as.empty()) {
                            as = "." + std::to_string((*next_variable_name)++);
                            add_definition(as, std::move(translated.first), definitions);
                        }
                        local_variable_renamings[name] = as;
                    } else {
                        names_bound_in_pattern.push_back(name);
                    }
                }

                auto alt_expr = translate_alt_expression(
                        alt.second,
                        next_variable_name,
                        local_variable_renamings,
                        data_constructor_arities,
                        names_bound_in_pattern,
                        definitions,
                        free_variables);

                if (alt.first->get_form() == patternform::literal) {
                    STGLiteral literal(dynamic_cast<LiteralPattern *>(alt.first.get())->value);
                    literal_alts.emplace_back(literal, std::move(alt_expr));
                } else if (alt.first->get_form() == patternform::wild) {
                    default_expr = std::move(alt_expr);
                    break;
                } else if (alt.first->get_form() == patternform::variable) {
                    if (first_alt_pattern_form != patternform::constructor) {
                        default_var = dynamic_cast<VariablePattern *>(alt.first.get())->name;
                    }
                    default_expr = std::move(alt_expr);
                    break;
                }
            }
        }

        std::vector<std::pair<STGPattern, std::unique_ptr<STGExpression>>> translated_alts;
        for (const auto &[constructor_name, alts]: constructor_alts) {
            std::vector<std::string> argument_variables;
            for (int i = 0; i < data_constructor_arities.at(constructor_name); i++) {
                argument_variables.push_back("." + std::to_string((*next_variable_name)++));
            }

            auto alt_expr = translate_case(
                    argument_variables,
                    alts,
                    argument_variables,
                    copy(default_expr),
                    next_variable_name,
                    data_constructor_arities,
                    definitions,
                    free_variables);

            translated_alts.emplace_back(
                    STGPattern(constructor_name, argument_variables),
                    std::move(alt_expr));
        }

        std::unique_ptr<STGExpression> case_expr;

        if (as.empty()) {
            free_variables.insert(
                    translated.first->free_variables.begin(),
                    translated.first->free_variables.end());
            case_expr = std::move(translated.first->expr);
        } else {
            free_variables.insert(as);
            case_expr = std::make_unique<STGVariable>(as);
        }

        if (first_alt_pattern_form == patternform::literal) {
            case_expr = std::make_unique<STGLiteralCase>(
                    std::move(case_expr),
                    std::move(literal_alts),
                    default_var,
                    std::move(default_expr));
        } else if (first_alt_pattern_form == patternform::constructor) {
            case_expr = std::make_unique<STGAlgebraicCase>(
                    std::move(case_expr),
                    std::move(translated_alts),
                    default_var,
                    std::move(default_expr));
        }

        return std::make_pair(
                std::make_unique<STGLambdaForm>(
                        free_variables,
                        std::vector<std::string>(),
                        true,
                        std::move(case_expr)),
                std::move(definitions));
    }
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
            std::string name = "." + std::to_string((*next_variable_name)++);
            add_definition(name, std::move(translated.first), definitions);
            argument_variables.insert(argument_variables.begin(), name);
        }

        expression = &(app->left);
    } while ((*expression)->get_form() == expform::application);

    if ((*expression)->get_form() == expform::constructor) {
        std::string constructor_name = dynamic_cast<Constructor*>(expression->get())->name;
        std::vector<std::string> additional_argument_variables;
        for (int i = argument_variables.size(); i < data_constructor_arities.at(constructor_name); i++) {
            additional_argument_variables.push_back("." + std::to_string((*next_variable_name)++));
        }
        std::vector<std::string> combined_argument_variables = argument_variables;
        combined_argument_variables.insert(
                combined_argument_variables.end(),
                additional_argument_variables.begin(),
                additional_argument_variables.end());
        return std::make_pair(
                std::make_unique<STGLambdaForm>(
                        std::set<std::string>(argument_variables.begin(), argument_variables.end()),
                        additional_argument_variables,
                        false,
                        std::make_unique<STGConstructor>(constructor_name, combined_argument_variables)),
                std::move(definitions));
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
        std::string name = "." + std::to_string((*next_variable_name)++);
        add_definition(name, std::move(translated.first), definitions);
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
    std::unique_ptr<STGExpression> body_expression = std::move(translated.first->expr);

    for (const auto &variable: argument_variables) {
        free_variables.erase(variable);
    }

    auto independent_definitions = capture_definitions_that_depend_on_names(
            std::move(translated.second),
            body_expression,
            free_variables,
            argument_variables);

    return std::make_pair(
            std::make_unique<STGLambdaForm>(
                    free_variables,
                    argument_variables,
                    false,
                    std::move(body_expression)),
            std::move(independent_definitions));
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
        case expform::builtinop:
            return translate_built_in_op(expr, next_variable_name, variable_renamings, data_constructor_arities);
        case expform::cAsE:
            return translate_case(expr, next_variable_name, variable_renamings, data_constructor_arities);
    }
}

void remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
        const std::unique_ptr<STGLambdaForm> &lambda_form,
        const std::set<std::string> &globals,
        const std::map<std::string, size_t> &number_of_arguments,
        std::set<std::string> &used_data_constructors);

void remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
        const std::unique_ptr<STGExpression> &expr,
        const std::set<std::string> &globals,
        const std::map<std::string, size_t> &number_of_arguments,
        std::set<std::string> &used_data_constructors) {
    if (expr->get_form() == stgform::let) {
        auto let = dynamic_cast<STGLet*>(expr.get());
        std::map<std::string, size_t> local_number_of_arguments = number_of_arguments;
        for (const auto &[name, lambda_form]: let->bindings) {
            local_number_of_arguments[name] = lambda_form->argument_variables.size();
        }
        for (const auto &[_, lambda_form]: let->bindings) {
            remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
                    lambda_form,
                    globals,
                    local_number_of_arguments,
                    used_data_constructors);
        }
        remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
                let->expr,
                globals,
                local_number_of_arguments,
                used_data_constructors);
    } else if (expr->get_form() == stgform::literalcase) {
        auto cAsE = dynamic_cast<STGLiteralCase*>(expr.get());
        remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
                cAsE->expr,
                globals,
                number_of_arguments,
                used_data_constructors);
        remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
                cAsE->default_expr,
                globals,
                number_of_arguments,
                used_data_constructors);
        for (const auto &[_, e]: cAsE->alts) {
            remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
                    e,
                    globals,
                    number_of_arguments,
                    used_data_constructors);
        }
    } else if (expr->get_form() == stgform::algebraiccase) {
        auto cAsE = dynamic_cast<STGAlgebraicCase*>(expr.get());
        remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
                cAsE->expr,
                globals,
                number_of_arguments,
                used_data_constructors);
        remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
                cAsE->default_expr,
                globals,
                number_of_arguments,
                used_data_constructors);
        for (const auto &[p, e]: cAsE->alts) {
            used_data_constructors.insert(p.constructor_name);
            std::map<std::string, size_t> local_number_of_arguments = number_of_arguments;
            for (const auto &v: p.variables) {
                local_number_of_arguments[v] = 0;
            }
            remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
                    e,
                    globals,
                    local_number_of_arguments,
                    used_data_constructors);
        }
    } else if (expr->get_form() == stgform::constructor) {
        used_data_constructors.insert(dynamic_cast<STGConstructor*>(expr.get())->constructor_name);
    }
}

void remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
        const std::unique_ptr<STGLambdaForm> &lambda_form,
        const std::set<std::string> &globals,
        const std::map<std::string, size_t> &number_of_arguments,
        std::set<std::string> &used_data_constructors) {
    if (lambda_form->expr->get_form() != stgform::constructor || !lambda_form->argument_variables.empty()) {
        for (auto it = lambda_form->free_variables.begin(); it != lambda_form->free_variables.end(); ) {
            if (globals.count(*it)) {
                it = lambda_form->free_variables.erase(it);
            } else {
                it++;
            }
        }
    }
    if (lambda_form->expr->get_form() == stgform::application) {
        auto application = dynamic_cast<STGApplication*>(lambda_form->expr.get());
        if (application->arguments.size() < number_of_arguments.at(application->lhs)) {
            lambda_form->updatable = false;
        }
    }

    std::map<std::string, size_t> local_number_of_arguments = number_of_arguments;

    for (const auto &v: lambda_form->argument_variables) {
        local_number_of_arguments[v] = 0;
    }

    remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
            lambda_form->expr,
            globals,
            local_number_of_arguments,
            used_data_constructors);
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

    std::map<std::string, size_t> number_of_arguments;
    std::set<std::string> globals;
    for (const auto &[name, lambda_form]: bindings) {
        number_of_arguments[name] = lambda_form->argument_variables.size();
        globals.insert(name);
    }

    std::map<std::string, std::unique_ptr<STGLambdaForm>> used_bindings;
    std::vector<std::string> to_add = {"main"};
    std::set<std::string> used_data_constructors;

    while (!to_add.empty()) {
        std::string name = to_add.back();
        to_add.pop_back();
        auto lambda_form = std::move(bindings.at(name));
        for (const auto &depends_on: lambda_form->free_variables) {
            if (
                    used_bindings.count(depends_on) == 0 &&
                    std::count(to_add.begin(), to_add.end(), depends_on) == 0 &&
                    depends_on != name &&
                    bindings.count(depends_on) > 0) {
                to_add.push_back(depends_on);
            }
        }
        remove_globals_from_free_variables_list_and_mark_partial_applications_as_non_updatable_and_collect_used_data_constructors(
                lambda_form,
                globals,
                number_of_arguments,
                used_data_constructors);
        used_bindings[name] = std::move(lambda_form);
    }

    std::map<std::string, STGDataConstructor> data_constructors;

    for (const auto &[_, type_constructor]: program->type_constructors) {
        for (unsigned int i = 0; i < type_constructor->data_constructors.size(); i++) {
            std::string name = type_constructor->data_constructors.at(i);
            if (used_data_constructors.count(name)) {
                size_t tag = i;
                if (name == "[]") {
                    tag = 0;
                } else if (name == ":") {
                    tag = 1;
                } else if (name == "False") {
                    tag = 0;
                } else if (name == "True") {
                    tag = 1;
                }
                size_t arity = program->data_constructor_arities.at(name);
                size_t number_of_siblings = type_constructor->data_constructors.size() - 1;
                data_constructors.emplace(name, STGDataConstructor(tag, arity, number_of_siblings));
            }
        }
    }

    return std::make_unique<STGProgram>(
            std::move(used_bindings),
            data_constructors);
}
