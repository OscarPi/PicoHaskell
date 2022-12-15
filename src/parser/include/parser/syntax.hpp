#ifndef PICOHASKELL_SYNTAX_HPP
#define PICOHASKELL_SYNTAX_HPP

#include <utility>
#include <map>
#include <string>
#include <memory>
#include <stdexcept>
#include <variant>
#include "types/types.hpp"

enum class patform {con, wild, lit, var};
enum class expform {var, con, lit, lam, app, cas, let, bop};
enum class builtinop {add, subtract, times, divide, equality, inequality, lt, lte, gt, gte, land, lor, negate};

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string &s): std::runtime_error(s) {}
};

class TConstructor;

class DConstructor {
private:
    const int lineNo;
    const std::string name;
    const std::vector<type> types;
    std::shared_ptr<TConstructor> tConstructor;
public:
    DConstructor(const int &lineNo, const std::string &name, const std::vector<type> &types);
    int getLineNo() const;
    std::string getName() const;
    std::vector<type> getTypes() const;
    void setTConstructor(const std::shared_ptr<TConstructor> &tConstructor);
    std::shared_ptr<TConstructor> getTConstructor() const;
};

class TConstructor {
private:
    const std::string name;
    const std::vector<std::string> argumentVariables;
    const std::vector<std::shared_ptr<DConstructor>> dConstructors;
    const int lineNo;
public:
    TConstructor(const int &lineNo, const std::string &name, const std::vector<std::string> &argumentVariables, const std::vector<std::shared_ptr<DConstructor>> &dConstructors);
    int getLineNo();
    std::string getName();
    std::vector<std::string> getArgumentVariables();
    std::vector<std::shared_ptr<DConstructor>> getDataConstructors();
};

struct Pattern {
    const int lineNo;
    const std::string as;
    explicit Pattern(const int &lineNo, const std::string &as): lineNo(lineNo), as(as) {}
    virtual patform getForm() = 0;
    virtual ~Pattern() = default;
};

struct ConPattern : Pattern {
    const std::string name;
    const std::vector<std::shared_ptr<Pattern>> args;
    ConPattern(
            const int &lineNo,
            const std::string &as,
            const std::string &name,
            const std::vector<std::shared_ptr<Pattern>> &args): Pattern(lineNo, as), name(name), args(args) {}
    patform getForm() override { return patform::con; }
};

struct WildPattern : Pattern {
    WildPattern(const int &lineNo, const std::string &as): Pattern(lineNo, as) {}
    patform getForm() override { return patform::wild; }
};

struct LiteralPattern : Pattern {
    const std::variant<int, std::string, char> value;
    LiteralPattern(const int &lineNo, const std::string &as, int value): Pattern(lineNo, as), value(value) {}
    LiteralPattern(const int &lineNo, const std::string &as, char value): Pattern(lineNo, as), value(value) {}
    LiteralPattern(const int &lineNo, const std::string &as, std::string value): Pattern(lineNo, as), value(value) {}
    patform getForm() override { return patform::lit; }
};

struct VarPattern : Pattern {
    const std::string name;
    VarPattern(const int &lineNo, const std::string &as, const std::string &name): Pattern(lineNo, as), name(name) {}
    patform getForm() override { return patform::var; }
};

struct Expression {
    const int lineNo;
    type signature;
    explicit Expression(const int &lineNo): lineNo(lineNo) {}
    virtual expform getForm() = 0;
    virtual ~Expression() = default;
};

struct Variable : public Expression {
    const std::string name;
    Variable(const int &lineNo, const std::string &name): Expression(lineNo), name(name) {}
    expform getForm() override { return expform::var; }
};

struct Constructor : public Expression {
    const std::string name;
    Constructor(const int &lineNo, const std::string &name): Expression(lineNo), name(name) {}
    expform getForm() override { return expform::con; }
};

struct Literal : public Expression {
    const std::variant<int, std::string, char> value;
    Literal(const int &lineNo, int i) : Expression(lineNo), value(i) {}
    Literal(const int &lineNo, char c) : Expression(lineNo), value(c) {}
    Literal(const int &lineNo, std::string s) : Expression(lineNo), value(s) {}
    expform getForm() override { return expform::lit; }
};

struct Lambda : public Expression {
    const std::vector<std::string> args;
    const std::shared_ptr<Expression> body;
    Lambda(
            const int &lineNo,
            const std::vector<std::string> &args,
            const std::shared_ptr<Expression> &body): Expression(lineNo), args(args), body(body) {}
    expform getForm() override { return expform::lam; }
};

struct Application : public Expression {
    const std::shared_ptr<Expression> left;
    const std::shared_ptr<Expression> right;
    Application(
            const int &lineNo,
            const std::shared_ptr<Expression> &left,
            const std::shared_ptr<Expression> &right): Expression(lineNo), left(left), right(right) {}
    expform getForm() override { return expform::app; }
};

struct Case : public Expression {
    const std::shared_ptr<Expression> exp;
    const std::vector<std::pair<std::shared_ptr<Pattern>, std::shared_ptr<Expression>>> alts;
    Case(
            const int &lineNo,
            const std::shared_ptr<Expression> &exp,
            const std::vector<std::pair<std::shared_ptr<Pattern>, std::shared_ptr<Expression>>> &alts
            ): Expression(lineNo), exp(exp), alts(alts) {}
    expform getForm() override { return expform::cas; }
};

struct Let : public Expression {
    expform getForm() override { return expform::let; }
};

struct BuiltInOp : public Expression {
    const std::shared_ptr<Expression> left;
    const std::shared_ptr<Expression> right;
    const builtinop op;
    BuiltInOp(
            const int &lineNo,
            const std::shared_ptr<Expression> &left,
            const std::shared_ptr<Expression> &right,
            const builtinop &op): Expression(lineNo), left(left), right(right), op(op) {}
    expform getForm() override { return expform::bop; }
};

struct Program {
    std::map<std::string, std::shared_ptr<TConstructor>> typeConstructors;
    std::map<std::string, std::shared_ptr<DConstructor>> dataConstructors;
    std::map<std::string, std::shared_ptr<Expression>> bindings;
    std::map<std::string, type> typeSignatures;
    void addTypeSignatures(const std::vector<std::string> &names, const type &t);
    void addTypeConstructor(
            int lineNo,
            const std::string &name,
            const std::vector<std::string> &argvars,
            const std::vector<std::shared_ptr<DConstructor>> &dConstructors);
    void addVariable(const int &lineNo, const std::string &name, const std::shared_ptr<Expression> &exp);
    void addNamedFunction(
            const int &lineNo,
            const std::string &name,
            const std::vector<std::string> &args,
            const std::shared_ptr<Expression> &body);
};

std::shared_ptr<Expression> makeIf(
        const int &lineNo,
        const std::shared_ptr<Expression> &e1,
        const std::shared_ptr<Expression> &e2,
        const std::shared_ptr<Expression> &e3);

std::shared_ptr<Expression> makeList(const int &lineNo, const std::vector<std::shared_ptr<Expression>> &elts);

#endif //PICOHASKELL_SYNTAX_HPP
