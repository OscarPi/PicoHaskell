#ifndef PICOHASKELL_STG_HPP
#define PICOHASKELL_STG_HPP

#include <utility>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <variant>
#include "parser/syntax.hpp"

enum class stgform {let, literal, variable, application, constructor, primitivecase, algebraiccase, primitiveop};

struct STGExpression {
    virtual stgform get_form() = 0;
    virtual ~STGExpression() = default;
};

struct STGLambdaForm {
    const std::set<std::string> free_variables;
    const std::set<std::string> argument_variables;
    const bool updatable;
    const std::unique_ptr<STGExpression> expr;
    STGLambdaForm(
            const std::set<std::string> &free_variables,
            const std::set<std::string> &argument_variables,
            const bool &updatable,
            std::unique_ptr<STGExpression> &&expr):
            free_variables(free_variables),
            argument_variables(argument_variables),
            updatable(updatable),
            expr(std::move(expr)) {}
};

struct STGLet : public STGExpression {
    std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
    std::unique_ptr<STGExpression> expr;
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
    STGLiteral(const int &i): value(i) {}
    STGLiteral(const char &c): value(c) {}
    stgform get_form() override { return stgform::literal; }
};

struct STGApplication : public STGExpression {
    const std::string lhs;
    const std::vector<std::string> arguments;
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
    const std::vector<std::pair<std::unique_ptr<STGLiteral>, std::unique_ptr<STGExpression>>> alts;
    const std::string default_var;
    const std::unique_ptr<STGExpression> default_expr;
    stgform get_form() override { return stgform::primitivecase; }
};

struct STGPattern {
    const std::string constructor_name;
    const std::vector<std::string> variables;
};

struct STGAlgebraicCase : public STGExpression {
    const std::unique_ptr<STGExpression> expr;
    const std::vector<std::pair<std::unique_ptr<STGPattern>, std::unique_ptr<STGExpression>>> alts;
    const std::string default_var;
    const std::unique_ptr<STGExpression> default_expr;
    stgform get_form() override { return stgform::algebraiccase; }
};

struct STGPrimitiveOp : public STGExpression {
    const std::string left;
    const std::string right;
    const builtinop op;
    stgform get_form() override { return stgform::primitiveop; }
};

struct STGProgram {
    const std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
    explicit STGProgram(
            std::map<std::string, std::unique_ptr<STGLambdaForm>> &&bindings): bindings(std::move(bindings)) {}
};

std::unique_ptr<STGProgram> translate(const std::unique_ptr<Program> &program);

#endif //PICOHASKELL_STG_HPP
