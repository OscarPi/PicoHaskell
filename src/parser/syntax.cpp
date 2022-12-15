#include <algorithm>
#include "parser/syntax.hpp"

DConstructor::DConstructor(const int &lineNo, const std::string &name, const std::vector<type> &types): lineNo(lineNo), name(name), types(types) {}

int DConstructor::getLineNo() const {
    return lineNo;
}

std::string DConstructor::getName() const {
    return name;
}

std::vector<type> DConstructor::getTypes() const {
    return types;
}

void DConstructor::setTConstructor(const std::shared_ptr<TConstructor> &constructor) {
    tConstructor = constructor;
    auto argvars = tConstructor->getArgumentVariables();
    for (auto t: getTypes()) {
        for (auto v: findTypeVariables(t)) {
            if (std::count(argvars.begin(), argvars.end(), v) == 0) {
                throw ParseError(
                        "Line " +
                        std::to_string(lineNo) +
                        ": unbound type variable."
                );
            }
        }
    }
}

std::shared_ptr<TConstructor> DConstructor::getTConstructor() const {
    return tConstructor;
}

TConstructor::TConstructor(const int &lineNo, const std::string &name,
                           const std::vector<std::string> &argumentVariables,
                           const std::vector<std::shared_ptr<DConstructor>> &dConstructors): lineNo(lineNo), name(name), argumentVariables(argumentVariables), dConstructors(dConstructors) {
    if (std::set<std::string>(argumentVariables.begin(), argumentVariables.end()).size() < argumentVariables.size()) {
        throw ParseError(
                "Line " +
                std::to_string(lineNo) +
                ": duplicate type variables not allowed."
        );
    }
}

std::string TConstructor::getName() {
    return name;
}

std::vector<std::string> TConstructor::getArgumentVariables() {
    return argumentVariables;
}

int TConstructor::getLineNo() {
    return lineNo;
}

std::vector<std::shared_ptr<DConstructor>> TConstructor::getDataConstructors() {
    return dConstructors;
}

void Program::addBinding(const std::string &name, const std::shared_ptr<Expression> &exp) {
    bindings[name] = exp;
}

void Program::addTypeSignatures(const std::vector<std::string> &names, const type &t) {
    for (const auto& name: names) {
        if (typeSignatures.count(name) > 0) {
            throw ParseError("Multiple type signatures for the same name are not allowed.");
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

void Program::addTypeConstructor(
        int lineNo,
        const std::string &name,
        const std::vector<std::string> &argvars,
        const std::vector<std::shared_ptr<DConstructor>> &dConstructors) {
    auto tConstructor = std::make_shared<TConstructor>(lineNo, name, argvars, dConstructors);
    for (const auto& constructor: dConstructors) {
        constructor->setTConstructor(tConstructor);
        if (dataConstructors.count(constructor->getName()) > 0) {
            throw ParseError(
                    "Line " +
                    std::to_string(constructor->getLineNo()) +
                    ": data constructor called " +
                    constructor->getName() +
                    " already exists."
            );
        }
        dataConstructors[constructor->getName()] = constructor;
    }
    if (typeConstructors.count(tConstructor->getName()) > 0) {
        throw ParseError(
                "Line " +
                std::to_string(tConstructor->getLineNo()) +
                ": type constructor called " +
                tConstructor->getName() +
                " already exists."
        );
    }
    typeConstructors[tConstructor->getName()] = tConstructor;
}

std::shared_ptr<TConstructor> Program::getTypeConstructor(const std::string &name) {
    return typeConstructors[name];
}

std::shared_ptr<DConstructor> Program::getDataConstructor(const std::string &name) {
    return dataConstructors[name];
}
