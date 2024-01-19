#ifndef MOSAIC_ECS_EXPR_H
#define MOSAIC_ECS_EXPR_H

#include <memory>
#include <variant>
#include <vector>

#include "token.h"
#include "value.h"

class Assign;
class CompoundAssign;
class Binary;
class Call;
class Literal;
class Logical;
class Set;
class Unary;
class Variable;

enum ExprType {
    EXPR_ASSIGN,
    EXPR_COMPOUND_ASSIGN,
    EXPR_BINARY,
    EXPR_CALL,
    EXPR_LITERAL,
    EXPR_LOGICAL,
    EXPR_SET,
    EXPR_UNARY,
    EXPR_VARIABLE,
};

class Expr {
public:
    virtual ~Expr() = default;
    bool is_type(ExprType type) const;

    template<class T>
    T& as() const {
        return *(T*)this;
    }

    template<class T>
    static std::shared_ptr<Expr> ptr(T expr) {
        return std::make_shared<T>(expr);
    }

    ExprType type;
};

using ExprPtr = std::shared_ptr<Expr>;

class Assign : public Expr {
public:
    Token name;
    ExprPtr value;
    Assign(Token name, ExprPtr value);
};

class CompoundAssign : public Expr {
public:
    Token name;
    Token op;
    ExprPtr value;
    CompoundAssign(Token name, Token op, ExprPtr value);
};

class Binary : public Expr {
public:
    ExprPtr left;
    Token op;
    ExprPtr right;
    Binary(ExprPtr left, Token op, ExprPtr right);
};

class Call : public Expr {
public:
    Token callee;
    std::vector<ExprPtr> arguments;
    Call(Token callee, std::vector<ExprPtr>& arguments);
};

class Literal : public Expr {
public:
    Token token;
    Value value;
    Literal(Token token, Value value);
};

class Logical : public Expr {
public:
    ExprPtr left;
    Token op;
    ExprPtr right;
    Logical(ExprPtr left, Token op, ExprPtr right);
};

class Set : public Expr {
public:
    ExprPtr object;
    Token name;
    ExprPtr value;
    Set(ExprPtr object, Token name, ExprPtr value);
};

class Unary : public Expr {
public:
    Token op;
    ExprPtr right;
    Unary(Token op, ExprPtr right);
};

class Variable : public Expr {
public:
    Token name;
    Variable(Token name);
};

std::ostream& operator<<(std::ostream& os, const Expr& expr);

#endif
