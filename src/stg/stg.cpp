#include <string>
#include <map>
#include <memory>
#include "stg/stg.hpp"
#include "parser/syntax.hpp"

std::map<std::string, std::unique_ptr<STGLambdaForm>> translate(const std::unique_ptr<Program> &program) {
    std::map<std::string, std::unique_ptr<STGLambdaForm>> stg_program;

    for (const auto &[name, expr]: program->bindings) {
        std::unique_ptr<STGLambdaForm> translated;
        switch(expr->get_form()) {
            case expform::variable:
                std::unique_ptr<STGExpression> e = std::make_unique<STGVariable>(
                        dynamic_cast<Variable*>(expr.get())->name);
                translated = std::make_unique<STGLambdaForm>(
                        std::vector<std::string>(),
                        std::vector<std::string>(),
                        true,
                        e);
                break;
        }
        stg_program[name] = std::move(translated);
    }

    return stg_program;
}
