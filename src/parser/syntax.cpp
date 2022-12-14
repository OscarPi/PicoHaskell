#include <stdexcept>
#include "parser/syntax.hpp"

void Program::addBinding(const std::string &name, const std::shared_ptr<Expression> &exp) {
    bindings[name] = exp;
}

void Program::addTypeSignatures(const std::vector<std::string> &names, const type &t) {
    for (const auto& name: names) {
        if (typeSignatures.count(name) > 0) {
            throw std::invalid_argument("Multiple type signatures for the same name are not allowed.");
        }
        typeSignatures[name] = t;
    }
}

std::shared_ptr<Expression> Program::getBinding(const std::string &name) {
    return bindings[name];
}

type Program::getTypeSignature(const std::string &name) {
    return typeSignatures[name];
}
