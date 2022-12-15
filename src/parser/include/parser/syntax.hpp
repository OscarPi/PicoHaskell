#ifndef PICOHASKELL_SYNTAX_HPP
#define PICOHASKELL_SYNTAX_HPP

#include <utility>
#include <map>
#include <string>
#include <memory>
#include <stdexcept>
#include "types/types.hpp"

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

class Expression {
public:
    virtual ~Expression() = default;
};

class Program {
private:
    std::map<std::string, std::shared_ptr<TConstructor>> typeConstructors;
    std::map<std::string, std::shared_ptr<DConstructor>> dataConstructors;
    std::map<std::string, std::shared_ptr<Expression>> bindings;
    std::map<std::string, type> typeSignatures;
public:
    void addBinding(const std::string &name, const std::shared_ptr<Expression> &exp);
    void addTypeSignatures(const std::vector<std::string> &names, const type &t);
    std::shared_ptr<Expression> getBinding(const std::string &name);
    type getTypeSignature(const std::string &name);
    void addTypeConstructor(
            int lineNo,
            const std::string &name,
            const std::vector<std::string> &argvars,
            const std::vector<std::shared_ptr<DConstructor>> &dConstructors);
    std::shared_ptr<TConstructor> getTypeConstructor(const std::string &name);
    std::shared_ptr<DConstructor> getDataConstructor(const std::string &name);
};

#endif //PICOHASKELL_SYNTAX_HPP
