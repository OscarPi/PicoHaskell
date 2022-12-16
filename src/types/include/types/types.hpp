#ifndef PICOHASKELL_TYPES_HPP
#define PICOHASKELL_TYPES_HPP
#include <memory>
#include <string>
#include <map>
#include <set>
#include <vector>

//enum class kindform {star, arrow};
enum class typeform {variable, constructor, application}; //, gen};

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

class Type;
typedef std::shared_ptr<const Type> type;
//typedef std::map<std::string, type> substitution;

struct Type {
    virtual ~Type() = default;
    virtual typeform get_form() const = 0;
};

struct TypeVariable : public Type {
    const std::string id;
    explicit TypeVariable(std::string id): id(id) {}
    typeform get_form() const override { return typeform::variable; }
};

struct TypeConstructor : public Type {
    const std::string id;
    explicit TypeConstructor(std::string id): id(id) {}
    typeform get_form() const override { return typeform::constructor; }
};

struct TypeApplication : public Type {
    const type left;
    const type right;
    TypeApplication(type left, type right): left(left), right(right) {}
    typeform get_form() const override { return typeform::application; }
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

const type tUnit = std::make_shared<const TypeConstructor>("()");
const type tChar = std::make_shared<const TypeConstructor>("Char");
const type tInt = std::make_shared<const TypeConstructor>("Int");
const type tFloat = std::make_shared<const TypeConstructor>("Float");
const type tDouble = std::make_shared<const TypeConstructor>("Double");

const type tList = std::make_shared<const TypeConstructor>("[]");
const type tArrow = std::make_shared<const TypeConstructor>("(->)");
const type tTuple2 = std::make_shared<const TypeConstructor>("(,)");

type make_function_type(const type &argType, const type &resultType);
type make_list_type(const type &elementType);
type make_pair_type(const type &leftType, const type &rightType);
//kind makeTupleConstructorKind(size_t size);
type make_tuple_type(const std::vector<type> &components);

//bool sameKind(const kind &a, const kind &b);
bool same_type(const type &a, const type &b);
//type applySubstitution(const type &t, const substitution &s);
//std::vector<std::string> findTypeVariables(const type &t);
//std::vector<type> applySubstitution(const std::vector<type> &ts, const substitution &s);
//std::vector<std::string> findTypeVariables(const std::vector<type> &ts);
//substitution compose(const substitution &s1, const substitution &s2);
//substitution merge(const substitution &s1, const substitution &s2);
//substitution mostGeneralUnifier(const type &t1, const type &t2);
//substitution match(const type &t1, const type &t2);

#endif //PICOHASKELL_TYPES_HPP
