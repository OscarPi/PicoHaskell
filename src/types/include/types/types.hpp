#ifndef PICOHASKELL_TYPES_HPP
#define PICOHASKELL_TYPES_HPP
#include <memory>
#include <string>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <stdexcept>

//enum class kindform {star, arrow};
enum class typeform {variable, universallyquantifiedvariable, constructor, application}; //, gen};

class TypeError : public std::runtime_error {
public:
    explicit TypeError(const std::string &s): std::runtime_error(s) {}
};

//class Kind {
//public:
//    virtual ~Kind() = default;
//    virtual kindform get_form() const = 0;
//};
//
//typedef std::shared_ptr<const Kind> kind;
//
//class StarKind : public Kind {
//public:
//    kindform get_form() const override;
//};
//
//class ArrowKind : public Kind {
//private:
//    const kind arg;
//    const kind result;
//public:
//    ArrowKind(kind arg, kind result);
//    kindform get_form() const override;
//    kind getArg() const;
//    kind getResult() const;
//};

//typedef std::map<std::string, type> substitution;

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
    typeform get_form() const override { return typeform::variable; }
};

//class TypeGeneric : public Type {
//private:
//    const int n;
//    const kind k;
//public:
//    TypeGeneric(int n, kind k);
//    typeform get_form() const override;
//    int getN() const;
//    kind getKind() const override;
//};
//
//class Scheme {
//private:
//    type t;
//public:
//    Scheme(const std::vector<std::shared_ptr<const TypeVariable>> &variables, const type &t);
//    explicit Scheme(type t);
//    Scheme applySubstitution(const substitution &s) const;
//    std::vector<std::string> findTypeVariables() const;
//    type getType() const;
//};
//bool operator==(const Scheme &lhs, const Scheme &rhs);
//bool operator!=(const Scheme &lhs, const Scheme &rhs);
//
//class Assumptions {
//private:
//    const std::map<std::string, Scheme> assumptions;
//    explicit Assumptions(const std::map<std::string, Scheme> &assumptions);
//public:
//    Assumptions() = default;
//    Assumptions add(const std::string &v, const Scheme &scheme) const;
//    Assumptions applySubstitution(const substitution &s) const;
//    std::vector<std::string> findTypeVariables() const;
//    Scheme find(const std::string &v) const;
//};

//const kind kStar = std::make_shared<const StarKind>();
//const kind kStarToStar = std::make_shared<const ArrowKind>(kStar, kStar);
//const kind kStarToStarToStar = std::make_shared<const ArrowKind>(kStar, kStarToStar);

Type *make_function_type(Type* const &argType, Type* const &resultType);
Type *make_list_type(Type* const &elementType);
//type make_pair_type(const type &leftType, const type &rightType);
//kind makeTupleConstructorKind(size_t size);
Type *make_tuple_type(const std::vector<Type*> &components);

//bool sameKind(const kind &a, const kind &b);
bool same_type(const Type *a, const Type *b);
//type applySubstitution(const type &t, const substitution &s);
//std::vector<std::string> findTypeVariables(const type &t);
//std::vector<type> applySubstitution(const std::vector<type> &ts, const substitution &s);
//std::vector<std::string> findTypeVariables(const std::vector<type> &ts);
//substitution compose(const substitution &s1, const substitution &s2);
//substitution merge(const substitution &s1, const substitution &s2);
//substitution mostGeneralUnifier(const type &t1, const type &t2);
//substitution match(const type &t1, const type &t2);

#endif //PICOHASKELL_TYPES_HPP
