#include <string>
#include <map>
#include <memory>
#include "stg/stg.hpp"
#include "parser/syntax.hpp"

std::unique_ptr<STGExpression> make_string(const std::string &s) {
    if (s.empty()) {
        return std::make_unique<STGConstructor>(
                "[]",
                std::vector<std::unique_ptr<STGAtom>>());
    }

    std::unique_ptr<STGExpression> str;

    for (int i = 0; i < s.length(); i++) {
        std::vector<std::unique_ptr<STGAtom>> args;
        args.push_back(std::make_unique<STGVariable>("#c" + std::to_string(i)));
        args.push_back(std::make_unique<STGVariable>("#t" + std::to_string(i)));
        std::unique_ptr<STGExpression> cons = std::make_unique<STGConstructor>(
                ":",
                std::move(args));

        args = std::vector<std::unique_ptr<STGAtom>>();
        args.push_back(std::make_unique<STGLiteral>(s[i]));
        std::unique_ptr<STGExpression> character = std::make_unique<STGConstructor>(
                "MakeChar",
                std::move(args));

        if (i == 0) {
            str = std::move(cons);
        } else {
            std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
            std::vector<std::string> free_variables = {"#c" + std::to_string(i), "#t" + std::to_string(i)};
            bindings["#t" + std::to_string(i-1)] = std::make_unique<STGLambdaForm>(
                    free_variables,
                    std::vector<std::string>(),
                    false,
                    std::move(cons));
            str = std::make_unique<STGLet>(
                    std::move(bindings),
                    std::move(str),
                    false);
        }

        std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
        bindings["#c" + std::to_string(i)] = std::make_unique<STGLambdaForm>(
                std::vector<std::string>(),
                std::vector<std::string>(),
                false,
                std::move(character));

        str = std::make_unique<STGLet>(
                std::move(bindings),
                std::move(str),
                false);
    }

    std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
    bindings["#t" + std::to_string(s.size() - 1)] = std::make_unique<STGLambdaForm>(
            std::vector<std::string>(),
            std::vector<std::string>(),
            false,
            std::make_unique<STGConstructor>(
                    "[]",
                    std::vector<std::unique_ptr<STGAtom>>()));
    str = std::make_unique<STGLet>(
            std::move(bindings),
            std::move(str),
            false);

    return str;
}

std::map<std::string, std::unique_ptr<STGLambdaForm>> translate(const std::unique_ptr<Program> &program) {
    std::map<std::string, std::unique_ptr<STGLambdaForm>> stg_program;

    for (const auto &[name, expr]: program->bindings) {
        std::unique_ptr<STGLambdaForm> translated;
        switch(expr->get_form()) {
            case expform::variable: {
                std::unique_ptr<STGExpression> e = std::make_unique<STGVariable>(
                        dynamic_cast<Variable*>(expr.get())->name);
                translated = std::make_unique<STGLambdaForm>(
                        std::vector<std::string>(),
                        std::vector<std::string>(),
                        true,
                        std::move(e));
                break;
            }
            case expform::literal: {
                std::variant<int, std::string, char> value = dynamic_cast<Literal*>(expr.get())->value;
                if (std::holds_alternative<int>(value)) {
                    std::vector<std::unique_ptr<STGAtom>> args;
                    args.push_back(std::make_unique<STGLiteral>(std::get<int>(value)));
                    translated = std::make_unique<STGLambdaForm>(
                            std::vector<std::string>(),
                            std::vector<std::string>(),
                            false,
                            std::make_unique<STGConstructor>(
                                    "MakeInt",
                                    std::move(args)));
                } else if (std::holds_alternative<std::string>(value)) {
                    translated = std::make_unique<STGLambdaForm>(
                            std::vector<std::string>(),
                            std::vector<std::string>(),
                            true,
                            make_string(std::get<std::string>(value)));
                } else if (std::holds_alternative<char>(value)) {
                    std::vector<std::unique_ptr<STGAtom>> args;
                    args.push_back(std::make_unique<STGLiteral>(std::get<char>(value)));
                    translated = std::make_unique<STGLambdaForm>(
                            std::vector<std::string>(),
                            std::vector<std::string>(),
                            false,
                            std::make_unique<STGConstructor>(
                                    "MakeChar",
                                    std::move(args)));
                }
                break;
            }
            case expform::constructor: {
                std::unique_ptr<STGExpression> e = std::make_unique<STGConstructor>(
                        dynamic_cast<Constructor*>(expr.get())->name,
                        std::vector<std::unique_ptr<STGAtom>>());
                translated = std::make_unique<STGLambdaForm>(
                        std::vector<std::string>(),
                        std::vector<std::string>(),
                        false,
                        std::move(e));
                break;
            }
        }
        stg_program[name] = std::move(translated);
    }

    return stg_program;
}
