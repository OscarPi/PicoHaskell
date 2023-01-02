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
    const std::vector<std::string> free_variables;
    const std::vector<std::string> argument_variables;
    const bool updatable;
    const std::unique_ptr<STGExpression> expr;
    STGLambdaForm(
            const std::vector<std::string> &free_variables,
            const std::vector<std::string> &argument_variables,
            const bool &updatable,
            std::unique_ptr<STGExpression> &&expr):
            free_variables(free_variables),
            argument_variables(argument_variables),
            updatable(updatable),
            expr(std::move(expr)) {}
};

struct STGAtom : public STGExpression {
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

struct STGLiteral : public STGAtom {
    const std::variant<int, char> value;
    STGLiteral(const int &i): value(i) {}
    STGLiteral(const char &c): value(c) {}
    stgform get_form() override { return stgform::literal; }
};

struct STGVariable : public STGAtom {
    const std::string name;
    explicit STGVariable(std::string name): name(std::move(name)) {}
    stgform get_form() override { return stgform::variable; }
};

struct STGApplication : public STGExpression {
    const std::string lhs;
    const std::vector<std::unique_ptr<STGAtom>> arguments;
    stgform get_form() override { return stgform::application; }
};

struct STGConstructor : public STGExpression {
    const std::string constructor_name;
    const std::vector<std::unique_ptr<STGAtom>> arguments;
    STGConstructor(
            std::string constructor_name,
            std::vector<std::unique_ptr<STGAtom>> &&arguments):
            constructor_name(std::move(constructor_name)),
            arguments(std::move(arguments)) {}
    stgform get_form() override { return stgform::constructor; }
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
    const std::unique_ptr<STGAtom> left;
    const std::unique_ptr<STGAtom> right;
    const builtinop op;
    stgform get_form() override { return stgform::primitiveop; }
};

std::map<std::string, std::unique_ptr<STGLambdaForm>> translate(const std::unique_ptr<Program> &program);

#endif //PICOHASKELL_STG_HPP
