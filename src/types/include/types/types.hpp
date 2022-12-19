#ifndef PICOHASKELL_TYPES_HPP
#define PICOHASKELL_TYPES_HPP
#include <memory>
#include <string>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <stdexcept>
#include "parser/syntax.hpp"

enum class typeform {variable, universallyquantifiedvariable, constructor, application}; //, gen};

class TypeError : public std::runtime_error {
public:
    explicit TypeError(const std::string &s): std::runtime_error(s) {}
};

struct Type {
    virtual ~Type() = default;
    virtual typeform get_form() const = 0;
};

struct UniversallyQuantifiedVariable : public Type {
    const std::string id;
    explicit UniversallyQuantifiedVariable(std::string id): id(std::move(id)) {}
    typeform get_form() const override { return typeform::universallyquantifiedvariable; }
};

struct TypeConstructor : public Type {
    const std::string id;
    explicit TypeConstructor(std::string id): id(std::move(id)) {}
    typeform get_form() const override { return typeform::constructor; }
};

struct TypeApplication : public Type {
    const std::shared_ptr<Type> left;
    const std::shared_ptr<Type> right;
    TypeApplication(Type* const &left, Type* const &right): left(left), right(right) {}
    TypeApplication(
            std::shared_ptr<Type> left,
            std::shared_ptr<Type> right): left(std::move(left)), right(std::move(right)) {}
    typeform get_form() const override { return typeform::application; }
};

struct TypeVariable : public Type {
    std::shared_ptr<Type> bound_to;
    std::string id;
    TypeVariable() { static int i = 0; id = std::to_string(i++); }
    typeform get_form() const override { return typeform::variable; }
};

Type *make_function_type(Type* const &argType, Type* const &resultType);
Type *make_list_type(Type* const &elementType);
Type *make_tuple_type(const std::vector<Type*> &components);

bool same_type(const Type *a, const Type *b);

//void type_check(const std::unique_ptr<Program> &program);

#endif //PICOHASKELL_TYPES_HPP
