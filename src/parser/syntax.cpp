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

void Program::addTypeSignatures(const std::vector<std::string> &names, const type &t) {
    for (const auto& name: names) {
        if (typeSignatures.count(name) > 0) {
            throw ParseError("Multiple type signatures for the same name are not allowed.");
        }
        typeSignatures[name] = t;
    }
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

void Program::addVariable(const int &lineNo, const std::string &name, const std::shared_ptr<Expression> &exp) {
    if (bindings.count(name) > 0) {
        throw ParseError(
                "Line " +
                std::to_string(lineNo) +
                ": multiple bindings to the name " +
                name +
                "."
        );
    }
    bindings[name] = exp;
}

void Program::addNamedFunction(const int &lineNo, const std::string &name, const std::vector<std::string> &args,
                               const std::shared_ptr<Expression> &body) {
    if (bindings.count(name) > 0) {
        throw ParseError(
                "Line " +
                std::to_string(lineNo) +
                ": multiple bindings to the name " +
                name +
                "."
        );
    }
    bindings[name] = std::make_shared<Lambda>(lineNo, args, body);
}

std::shared_ptr<Expression> makeIf(
        const int &lineNo,
        const std::shared_ptr<Expression> &e1,
        const std::shared_ptr<Expression> &e2,
        const std::shared_ptr<Expression> &e3) {
    const auto t = std::make_shared<ConPattern>(lineNo, "", "True", std::vector<std::shared_ptr<Pattern>>());
    const auto f = std::make_shared<ConPattern>(lineNo, "", "False", std::vector<std::shared_ptr<Pattern>>());
    const auto alt1 = std::make_pair(t, e2);
    const auto alt2 = std::make_pair(f, e3);
    const std::vector<std::pair<std::shared_ptr<Pattern>, std::shared_ptr<Expression>>> alts = {alt1, alt2};
    return std::make_shared<Case>(lineNo, e1, alts);
}
