#include "expr.h"

std::ostream& operator<<(std::ostream& os, const Expr& expr) {
    switch (expr.type) {
        case EXPR_ASSIGN: os << "Assign(" << expr.as<Assign>().name.lexeme << ", " << *expr.as<Assign>().value << ")"; break;
        case EXPR_COMPOUND_ASSIGN: {
            CompoundAssign& comp_assign = expr.as<CompoundAssign>();
            switch (comp_assign.op.type) {
                case TOKEN_PLUS_EQUAL: os << "AddEqual("; break;
                case TOKEN_MINUS_EQUAL: os << "SubtractEqual("; break;
                case TOKEN_STAR_EQUAL: os << "MultiplyEqual("; break;
                case TOKEN_SLASH_EQUAL: os << "DivideEqual("; break;
            }
            os << comp_assign.name.lexeme << ", " << *comp_assign.value << ")";
            break;
        }
        case EXPR_BINARY: {
            Binary& binary = expr.as<Binary>();
            switch (binary.op.type) {
                case TOKEN_PLUS: os << "Add("; break;
                case TOKEN_MINUS: os << "Subtract("; break;
                case TOKEN_STAR: os << "Multiply("; break;
                case TOKEN_SLASH: os << "Divide("; break;
                case TOKEN_EQUAL_EQUAL: os << "Equal("; break;
                case TOKEN_BANG_EQUAL: os << "!Equal("; break;
                case TOKEN_LESS: os << "Less("; break;
                case TOKEN_LESS_EQUAL: os << "LessEqual("; break;
                case TOKEN_GREATER: os << "Greater("; break;
                case TOKEN_GREATER_EQUAL: os << "GreaterEqual("; break;
                default: os << "ERROR";
            }
            os << *binary.left << ", " << *binary.right << ")";
            break;
        }
        case EXPR_CALL: {
            Call& call = expr.as<Call>();
            os << "Call(" << call.callee.lexeme << "(";
            for (int i = 0; i < call.arguments.size(); i++) {
                os << *call.arguments[i];
                if (i + 1 < call.arguments.size()) os << ", ";
            }
            os << "))";
            break;
        }
        case EXPR_LITERAL: os << expr.as<Literal>().token.lexeme; break;
        case EXPR_LOGICAL: {
            Logical& logical = expr.as<Logical>();
            switch (logical.op.type) {
                case TOKEN_OR: os << "Or("; break;
                case TOKEN_AND: os << "And("; break;
            }
            os << *logical.left << ", " << *logical.right << ")";
            break;
        }
        case EXPR_SET: os << "Set(" << expr.as<Set>().name.lexeme << ", " << *expr.as<Set>().value << ")"; break;
        case EXPR_VARIABLE: os << "Variable(" << expr.as<Variable>().name.lexeme << ")"; break;
    }
    return os;
}

bool Expr::is_type(ExprType type) const {
    return this->type == type;
}

Assign::Assign(Token name, ExprPtr value) {
    this->type = EXPR_ASSIGN;
    this->name = name;
    this->value = value;
}

CompoundAssign::CompoundAssign(Token name, Token op, ExprPtr value) {
    this->type = EXPR_COMPOUND_ASSIGN;
    this->name = name;
    this->op = op;
    this->value = value;
}

Binary::Binary(ExprPtr left, Token op, ExprPtr right) {
    this->type = EXPR_BINARY;
    this->left = left;
    this->op = op;
    this->right = right;
}

Call::Call(Token callee, std::vector<ExprPtr> &arguments) {
    this->type = EXPR_CALL;
    this->callee = callee;
    this->arguments = arguments;
}

Literal::Literal(Token token, Value value) {
    this->type = EXPR_LITERAL;
    this->token = token;
    this->value = value;
}

Logical::Logical(ExprPtr left, Token op, ExprPtr right) {
    this->type = EXPR_LOGICAL;
    this->left = left;
    this->op = op;
    this->right = right;
}

Set::Set(ExprPtr object, Token name, ExprPtr value) {
    this->type = EXPR_SET;
    this->object = object;
    this->name = name;
    this->value = value;
}

Unary::Unary(Token op, ExprPtr right) {
    this->type = EXPR_UNARY;
    this->op = op;
    this->right = right;
}

Variable::Variable(Token name) {
    this->type = EXPR_VARIABLE;
    this->name = name;
}
