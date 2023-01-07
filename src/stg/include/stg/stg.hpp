#ifndef PICOHASKELL_STG_HPP
#define PICOHASKELL_STG_HPP

#include <utility>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <variant>
#include "parser/syntax.hpp"

enum class stgform {let, literal, variable, application, constructor, literalcase, algebraiccase, primitiveop};

struct STGExpression {
    virtual stgform get_form() = 0;
    virtual ~STGExpression() = default;
};

struct STGLambdaForm {
    std::set<std::string> free_variables;
    const std::vector<std::string> argument_variables;
    bool updatable;
    std::unique_ptr<STGExpression> expr;
    STGLambdaForm(
            const std::set<std::string> &free_variables,
            const std::vector<std::string> &argument_variables,
            const bool &updatable,
            std::unique_ptr<STGExpression> &&expr):
            free_variables(free_variables),
            argument_variables(argument_variables),
            updatable(updatable),
            expr(std::move(expr)) {}
};

struct STGLet : public STGExpression {
    const std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
    const std::unique_ptr<STGExpression> expr;
    const bool recursive;
    STGLet(
            std::map<std::string, std::unique_ptr<STGLambdaForm>> &&bindings,
            std::unique_ptr<STGExpression> &&expr,
            const bool &recursive):
            bindings(std::move(bindings)),
            expr(std::move(expr)),
            recursive(recursive) {}
    stgform get_form() override { return stgform::let; }
};

struct STGLiteral : public STGExpression {
    const std::variant<int, char> value;
    explicit STGLiteral(const std::variant<int, char> &value): value(value) {}
    stgform get_form() override { return stgform::literal; }
};

struct STGApplication : public STGExpression {
    const std::string lhs;
    const std::vector<std::string> arguments;
    STGApplication(
            std::string lhs,
            const std::vector<std::string> &arguments): lhs(std::move(lhs)), arguments(arguments) {}
    stgform get_form() override { return stgform::application; }
};

struct STGConstructor : public STGExpression {
    const std::string constructor_name;
    const std::vector<std::string> arguments;
    explicit STGConstructor(std::string constructor_name): constructor_name(std::move(constructor_name)) {}
    STGConstructor(
            std::string constructor_name,
            const std::vector<std::string> &arguments):
            constructor_name(std::move(constructor_name)),
            arguments(arguments) {}
    stgform get_form() override { return stgform::constructor; }
};

struct STGVariable : public STGExpression {
    const std::string name;
    explicit STGVariable(std::string name): name(std::move(name)) {}
    stgform get_form() override { return stgform::variable; }
};

struct STGLiteralCase : public STGExpression {
    const std::unique_ptr<STGExpression> expr;
    const std::vector<std::pair<STGLiteral, std::unique_ptr<STGExpression>>> alts;
    const std::string default_var;
    const std::unique_ptr<STGExpression> default_expr;
    STGLiteralCase(
            std::unique_ptr<STGExpression> &&expr,
            std::vector<std::pair<STGLiteral, std::unique_ptr<STGExpression>>> &&alts,
            std::string default_var,
            std::unique_ptr<STGExpression> &&default_expr):
            expr(std::move(expr)),
            alts(std::move(alts)),
            default_var(std::move(default_var)),
            default_expr(std::move(default_expr)) {}
    stgform get_form() override { return stgform::literalcase; }
};

struct STGPattern {
    const std::string constructor_name;
    const std::vector<std::string> variables;
    STGPattern(
            std::string constructor_name,
            const std::vector<std::string> &variables):
            constructor_name(std::move(constructor_name)),
            variables(variables) {}
};

struct STGAlgebraicCase : public STGExpression {
    const std::unique_ptr<STGExpression> expr;
    const std::vector<std::pair<STGPattern, std::unique_ptr<STGExpression>>> alts;
    const std::string default_var;
    const std::unique_ptr<STGExpression> default_expr;
    STGAlgebraicCase(
            std::unique_ptr<STGExpression> &&expr,
            std::vector<std::pair<STGPattern, std::unique_ptr<STGExpression>>> &&alts,
            std::string default_var,
            std::unique_ptr<STGExpression> &&default_expr):
            expr(std::move(expr)),
            alts(std::move(alts)),
            default_var(std::move(default_var)),
            default_expr(std::move(default_expr)) {}
    stgform get_form() override { return stgform::algebraiccase; }
};

struct STGPrimitiveOp : public STGExpression {
    const std::string left;
    const std::string right;
    const builtinop op;
    STGPrimitiveOp(
            std::string left,
            std::string right,
            const builtinop &op): left(std::move(left)), right(std::move(right)), op(op) {}
    stgform get_form() override { return stgform::primitiveop; }
};

struct STGProgram {
    const std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
    explicit STGProgram(
            std::map<std::string, std::unique_ptr<STGLambdaForm>> &&bindings): bindings(std::move(bindings)) {}
};

std::unique_ptr<STGProgram> translate(const std::unique_ptr<Program> &program);

#endif //PICOHASKELL_STG_HPP
