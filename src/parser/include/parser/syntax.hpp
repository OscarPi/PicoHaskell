#ifndef PICOHASKELL_SYNTAX_HPP
#define PICOHASKELL_SYNTAX_HPP

#include <utility>
#include <map>
#include <string>
#include <memory>
#include <stdexcept>
#include <variant>
#include "types/types.hpp"

enum class patternform {constructor, wild, literal, variable};
enum class expform {variable, constructor, literal, abstraction, application, cAsE, let, builtinop};
enum class builtinop {add, subtract, times, divide, equality, inequality, lt, lte, gt, gte, land, lor, negate};

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string &s): std::runtime_error(s) {}
};

struct DConstructor {
    const int line;
    const std::string name;
    std::vector<std::shared_ptr<Type>> types;
    std::string type_constructor;
    DConstructor(const int &line, std::string name, const std::vector<Type*> &types);
};

struct TConstructor {
    const std::string name;
    const std::vector<std::string> argument_variables;
    const std::vector<std::string> data_constructors;
    const int line;
    TConstructor(
            const int &line,
            std::string name,
            const std::vector<std::string> &argument_variables,
            const std::vector<std::string> &data_constructors):
            line(line),
            name(std::move(name)),
            argument_variables(argument_variables),
            data_constructors(data_constructors) {}
};

struct Pattern {
    const int line;
    std::vector<std::string> as;
    explicit Pattern(const int &line): line(line) {}
    virtual patternform get_form() = 0;
    virtual ~Pattern() = default;
};

struct ConstructorPattern : Pattern {
    const std::string name;
    std::vector<std::unique_ptr<Pattern>> args;
    ConstructorPattern(const int &line, std::string name, const std::vector<Pattern*> &args);
    patternform get_form() override { return patternform::constructor; }
};

struct WildPattern : Pattern {
    WildPattern(const int &line): Pattern(line) {}
    patternform get_form() override { return patternform::wild; }
};

struct LiteralPattern : Pattern {
    const std::variant<int, std::string, char> value;
    LiteralPattern(const int &line, int value): Pattern(line), value(value) {}
    LiteralPattern(const int &line, char value): Pattern(line), value(value) {}
    LiteralPattern(const int &line, std::string value): Pattern(line), value(value) {}
    patternform get_form() override { return patternform::literal; }
};

struct VariablePattern : Pattern {
    const std::string name;
    VariablePattern(const int &line, std::string name): Pattern(line), name(std::move(name)) {}
    patternform get_form() override { return patternform::variable; }
};

struct Expression {
    const int line;
    explicit Expression(const int &line): line(line) {}
    virtual expform get_form() = 0;
    virtual ~Expression() = default;
};

struct Variable : public Expression {
    const std::string name;
    Variable(const int &line, std::string name): Expression(line), name(std::move(name)) {}
    expform get_form() override { return expform::variable; }
};

struct Constructor : public Expression {
    const std::string name;
    Constructor(const int &line, std::string name): Expression(line), name(std::move(name)) {}
    expform get_form() override { return expform::constructor; }
};

struct Literal : public Expression {
    const std::variant<int, std::string, char> value;
    Literal(const int &line, const int &i): Expression(line), value(i) {}
    Literal(const int &line, const char &c): Expression(line), value(c) {}
    Literal(const int &line, std::string s): Expression(line), value(s) {}
    expform get_form() override { return expform::literal; }
};

struct Abstraction : public Expression {
    const std::vector<std::string> args;
    const std::unique_ptr<Expression> body;
    Abstraction(
            const int &line,
            const std::vector<std::string> &args,
            Expression * const &body): Expression(line), args(args), body(body) {}
    expform get_form() override { return expform::abstraction; }
};

struct Application : public Expression {
    const std::unique_ptr<Expression> left;
    const std::unique_ptr<Expression> right;
    Application(
            const int &line,
            Expression * const &left,
            Expression * const &right): Expression(line), left(left), right(right) {}
    expform get_form() override { return expform::application; }
};

struct Case : public Expression {
    const std::unique_ptr<Expression> exp;
    std::vector<std::pair<std::unique_ptr<Pattern>, std::unique_ptr<Expression>>> alts;
    Case(const int &line, Expression * const &exp, const std::vector<std::pair<Pattern*, Expression*>> &alts);
    expform get_form() override { return expform::cAsE; }
};

struct Let : public Expression {
    std::map<std::string, std::unique_ptr<Expression>> bindings;
    std::map<std::string, std::shared_ptr<Type>> type_signatures;
    const std::unique_ptr<Expression> e;
    Let(
            const int &line,
            const std::map<std::string, Expression*> &bindings,
            const std::map<std::string, Type*> &type_signatures,
            Expression * const &e
            );
    expform get_form() override { return expform::let; }
};

struct BuiltInOp : public Expression {
    const std::unique_ptr<Expression> left;
    const std::unique_ptr<Expression> right;
    const builtinop op;
    BuiltInOp(
            const int &line,
            Expression * const &left,
            Expression * const &right,
            const builtinop &op): Expression(line), left(left), right(right), op(op) {}
    expform get_form() override { return expform::builtinop; }
};

struct Program {
    std::map<std::string, std::unique_ptr<TConstructor>> type_constructors;
    std::map<std::string, std::unique_ptr<DConstructor>> data_constructors;
    std::map<std::string, std::unique_ptr<Expression>> bindings;
    std::map<std::string, std::shared_ptr<Type>> type_signatures;
    Program();
    void add_type_signature(const int &line, const std::string &name, Type* const &t);
    void add_type_constructor(
            const int &line,
            const std::string &name,
            const std::vector<std::string> &argument_variables,
            const std::vector<DConstructor*> &new_data_constructors);
    void add_variable(const int &line, const std::string &name, Expression * const &exp);
    void add_named_function(
            const int &line,
            const std::string &name,
            const std::vector<std::string> &args,
            Expression * const &body);
};

typedef std::vector<std::pair<std::string, Type*>> typesigs;
typedef std::vector<std::tuple<std::string, std::vector<std::string>, Expression*>> funcs;
typedef std::vector<std::pair<std::string, Expression*>> vars;
typedef std::tuple<typesigs, funcs, vars> declist;

Expression *make_if_expression(const int &line, Expression* const &e1, Expression* const &e2, Expression* const &e3);
Expression *make_list_expression(const int &line, const std::vector<Expression*> &elements);
Expression *make_tuple_expression(const int &line, const std::vector<Expression*> &elements);
Expression *make_let_expression(const int &line, const declist &decls, Expression* const &e);
Pattern *make_list_pattern(const int &line, const std::vector<Pattern*> &elements);
Pattern *make_tuple_pattern(const int &line, const std::vector<Pattern*> &elements);

#endif //PICOHASKELL_SYNTAX_HPP
