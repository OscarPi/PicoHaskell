#ifndef PICOHASKELL_STG_HPP
#define PICOHASKELL_STG_HPP

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <variant>
#include "parser/syntax.hpp"

struct STGExpression {

};

struct STGLambdaForm {
    const std::vector<std::string> free_variables;
    const std::vector<std::string> argument_variables;
    const bool updatable;
    const std::unique_ptr<STGExpression> expr;
};

struct STGAtom : public STGExpression {

};

struct STGLet : public STGExpression {
    const std::map<std::string, std::unique_ptr<STGLambdaForm>> bindings;
    const std::unique_ptr<STGExpression> expr;
    const bool recursive;
};

struct STGLiteral : public STGAtom {
    const std::variant<int, std::string, char> value;
};

struct STGVariable : public STGAtom {
    const std::string name;
};

struct STGApplication : public STGExpression {
    const std::string lhs;
    const std::vector<std::unique_ptr<STGAtom>> arguments;
};

struct STGConstructor : public STGExpression {
    const std::string constructor_name;
    const std::vector<std::unique_ptr<STGAtom>> arguments;
};

struct STGPrimitiveCase : public STGExpression {
    const std::unique_ptr<STGExpression> expr;
    const std::vector<std::pair<std::unique_ptr<STGLiteral>, std::unique_ptr<STGExpression>>> alts;
};

struct STGPattern {
    const std::string constructor_name;
    const std::vector<std::string> variables;
};

struct STGAlgebraicCase : public STGExpression {
    const std::unique_ptr<STGExpression> expr;
    const std::vector<std::pair<std::unique_ptr<STGPattern>, std::unique_ptr<STGExpression>>> alts;
};

struct STGPrimitiveOp : public STGExpression {
    const std::unique_ptr<STGAtom> left;
    const std::unique_ptr<STGAtom> right;
    const builtinop op;
};

std::map<std::string, std::unique_ptr<STGLambdaForm>> translate(const std::unique_ptr<Program> &program);

#endif //PICOHASKELL_STG_HPP
