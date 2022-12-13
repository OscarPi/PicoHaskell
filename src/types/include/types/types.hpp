#ifndef PICOHASKELL_TYPES_HPP
#define PICOHASKELL_TYPES_HPP
#include <memory>
#include <string>
#include <map>
#include <set>
#include <vector>

enum class kindtype {star, arrow};
enum class ttype {var, con, ap, gen};

class Kind {
public:
    virtual ~Kind() = default;
    virtual kindtype getType() const = 0;
};

typedef std::shared_ptr<const Kind> kind;

class StarKind : public Kind {
public:
    kindtype getType() const override;
};

class ArrowKind : public Kind {
private:
    const kind arg;
    const kind result;
public:
    ArrowKind(kind arg, kind result);
    kindtype getType() const override;
    kind getArg() const;
    kind getResult() const;
};

class Type;
typedef std::shared_ptr<const Type> type;
typedef std::map<std::string, type> substitution;

class Type {
public:
    virtual ~Type() = default;
    virtual ttype getType() const = 0;
    virtual kind getKind() const = 0;
};

class TypeVariable : public Type {
private:
    const std::string id;
    const kind k;
public:
    TypeVariable(std::string id, kind k);
    ttype getType() const override;
    std::string getId() const;
    kind getKind() const override;
};

class TypeConstructor : public Type {
private:
    const std::string id;
    const kind k;
public:
    TypeConstructor(std::string id, kind k);
    ttype getType() const override;
    std::string getId() const;
    kind getKind() const override;
};

class TypeApplication : public Type {
private:
    const type left;
    const type right;
public:
    TypeApplication(type left, type right);
    ttype getType() const override;
    type getLeft() const;
    type getRight() const;
    kind getKind() const override;
};

class TypeGeneric : public Type {
private:
    const int n;
public:
    explicit TypeGeneric(int n);
    ttype getType() const override;
    int getN() const;
    kind getKind() const override;
};

const kind kStar = std::make_shared<const StarKind>();
const kind kStarToStar = std::make_shared<const ArrowKind>(kStar, kStar);
const kind kStarToStarToStar = std::make_shared<const ArrowKind>(kStar, kStarToStar);

const type tUnit = std::make_shared<const TypeConstructor>("()", kStar);
const type tChar = std::make_shared<const TypeConstructor>("Char", kStar);
const type tInt = std::make_shared<const TypeConstructor>("Int", kStar);
const type tFloat = std::make_shared<const TypeConstructor>("Float", kStar);
const type tDouble = std::make_shared<const TypeConstructor>("Double", kStar);

const type tList = std::make_shared<const TypeConstructor>("[]", kStarToStar);
const type tArrow = std::make_shared<const TypeConstructor>("(->)", kStarToStarToStar);
const type tTuple2 = std::make_shared<const TypeConstructor>("(,)", kStarToStarToStar);

type makeFunctionType(const type &argType, const type &resultType);
type makeListType(const type &elementType);
type makePairType(const type &leftType, const type &rightType);

bool sameKind(const kind &a, const kind &b);
bool sameType(const type &a, const type &b);
type applySubstitution(const type &t, substitution s);
std::set<std::string> findTypeVariables(const type &t);
std::vector<type> applySubstitution(const std::vector<type> &ts, substitution s);
std::set<std::string> findTypeVariables(const std::vector<type> &ts);
substitution compose(const substitution &s1, const substitution &s2);

#endif //PICOHASKELL_TYPES_HPP
