#ifndef MOSAIC_ECS_STMT_H
#define MOSAIC_ECS_STMT_H

#include <variant>
#include <vector>

#include "expr.h"

enum StmtType {
    STMT_BLOCK,
    STMT_EXPR,
    STMT_FUN,
    STMT_IF,
    STMT_LET,
    STMT_PRINT,
    STMT_RETURN,
    STMT_WHILE,
};

class Stmt {
public:
    virtual ~Stmt() = default;
    bool is_type(StmtType type) const;

    template<class T>
    T& as() const {
        return *(T*)this;
    }

    template<class T>
    static std::shared_ptr<Stmt> ptr(T stmt) {
        return std::make_shared<T>(stmt);
    }

    StmtType type;
};

using StmtPtr = std::shared_ptr<Stmt>;

class Block : public Stmt {
public:
    std::vector<StmtPtr> stmts;
    Block(std::vector<StmtPtr>& stmts);
};

class ExprStmt : public Stmt {
public:
    ExprPtr expr;
    ExprStmt(ExprPtr expr);
};

class FunStmt : public Stmt {
public:
    Token name;
    std::vector<Token> parameters;
    StmtPtr body;
    FunStmt(Token name, std::vector<Token>& parameters, StmtPtr body);
};

class If : public Stmt {
public:
    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch;
    If(ExprPtr condition, StmtPtr then_branch, StmtPtr else_branch);
};


class Let : public Stmt {
public:
    Token name;
    ExprPtr initializer;
    Let(Token name, ExprPtr initializer);
};

class Print : public Stmt {
public:
    ExprPtr value;
    Print(ExprPtr value);
};

class Return : public Stmt {
public:
    Token debug;
    ExprPtr value;
    Return(Token debug, ExprPtr value);
};

class While : public Stmt {
public:
    ExprPtr condition;
    StmtPtr body;
    While(ExprPtr condition, StmtPtr body);
};

std::ostream& operator<<(std::ostream& os, const Stmt& stmt);

#endif
