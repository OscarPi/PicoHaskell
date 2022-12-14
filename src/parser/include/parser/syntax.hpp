#ifndef PICOHASKELL_SYNTAX_HPP
#define PICOHASKELL_SYNTAX_HPP

#include <utility>
#include <map>
#include <string>
#include <memory>
#include "types/types.hpp"


class Expression {
public:
    virtual ~Expression() = default;
};

class Program {
private:
    std::map<std::string, std::shared_ptr<Expression>> bindings;
    std::map<std::string, type> typeSignatures;
public:
    void addBinding(const std::string &name, const std::shared_ptr<Expression> &exp);
    void addTypeSignatures(const std::vector<std::string> &names, const type &t);
    std::shared_ptr<Expression> getBinding(const std::string &name);
    type getTypeSignature(const std::string &name);
};

#endif //PICOHASKELL_SYNTAX_HPP
