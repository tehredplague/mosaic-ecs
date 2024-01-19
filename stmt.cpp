#include <iostream>
#include "stmt.h"

std::ostream& operator<<(std::ostream& os, const Stmt& stmt) {
    switch (stmt.type) {
        case STMT_BLOCK: {
            Block& block = stmt.as<Block>();
            os << "Block(";
            for (int i = 0; i < block.stmts.size(); i++) {
                os << *block.stmts[i];
                if (i + 1 < block.stmts.size()) os << ", ";
            }
            os << ")";
            break;
        }
        case STMT_EXPR: os << *stmt.as<ExprStmt>().expr; break;
        case STMT_FUN: {
            FunStmt& fun = stmt.as<FunStmt>();
            os << "Fun(" << "<fn " << fun.name.lexeme << ">(";
            for (int i = 0; i < fun.parameters.size(); i++) {
                os << fun.parameters[i].lexeme;
                if (i + 1 < fun.parameters.size()) os << ", ";
            }
            os << "), " << *fun.body << ")";
            break;
        }
        case STMT_IF: {
            If& if_stmt = stmt.as<If>();
            os << "If(" << *if_stmt.condition << ", Then(" << *if_stmt.then_branch << ")";
            if (if_stmt.else_branch) os << ", Else(" << *if_stmt.else_branch << ")";
            os << ")";
            break;
        }
        case STMT_LET: os << "Let(" << stmt.as<Let>().name.lexeme << ", " << *stmt.as<Let>().initializer << ")"; break;
        case STMT_PRINT: os << "Print(" << *stmt.as<Print>().value << ")"; break;
        case STMT_RETURN: os << "Return(" << *stmt.as<Return>().value << ")"; break;
        case STMT_WHILE: os << "While(" << *stmt.as<While>().condition << ", " << *stmt.as<While>().body << ")"; break;
    }
    return os;
}

bool Stmt::is_type(StmtType type) const {
    return this->type == type;
}

Block::Block(std::vector<StmtPtr> &stmts) {
    this->type = STMT_BLOCK;
    this->stmts = stmts;
}

ExprStmt::ExprStmt(ExprPtr expr) {
    this->type = STMT_EXPR;
    this->expr = expr;
}

FunStmt::FunStmt(Token name, std::vector<Token>& parameters, StmtPtr body) {
    this->type = STMT_FUN;
    this->name = name;
    this->parameters = parameters;
    this->body = body;
}

If::If(ExprPtr condition, StmtPtr then_branch, StmtPtr else_branch) {
    this->type = STMT_IF;
    this->condition = condition;
    this->then_branch = then_branch;
    this->else_branch = else_branch;
}

Let::Let(Token name, ExprPtr initializer) {
    this->type = STMT_LET;
    this->name = name;
    this->initializer = initializer;
}

Print::Print(ExprPtr value) {
    this->type = STMT_PRINT;
    this->value = value;
}

Return::Return(Token debug, ExprPtr value) {
    this->type = STMT_RETURN;
    this->debug = debug;
    this->value = value;
}

While::While(ExprPtr condition, StmtPtr body) {
    this->type = STMT_WHILE;
    this->condition = condition;
    this->body = body;
}
