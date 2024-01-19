#include "debug.h"
#include "parser.h"

Parser::Parser(std::vector<Token> tokens) : tokens(tokens), current(0), scope_depth(0) {
    panic_mode = false;
    had_error = false;
}

std::vector<StmtPtr> Parser::parse() {
    while (!is_at_end()) {
        try {
            stmts.push_back(declaration());
        } catch (const std::exception& e) {
            std::cerr << "[line " << previous().line << "] " << e.what() << std::endl;
            exit(-1);
        }
    }
#ifdef STMT_DEBUG
    std::cout << "==<AST>==" << std::endl;
    for (auto stmt : stmts) {
        std::cout << *stmt << std::endl;
    }
#endif
    return stmts;
}

StmtPtr Parser::declaration() {
    if (match(TOKEN_FUN)) return fun_declaration();
    if (match(TOKEN_LET)) return let_declaration();
    else return statement();
}

StmtPtr Parser::fun_declaration() {
    consume(TOKEN_IDENTIFIER, "Expect function_index name.");
    Token name = previous();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after function_index name.");
    std::vector<Token> parameters;
    if (check(TOKEN_IDENTIFIER)) {
        do {
            if (parameters.size() >= 255) {
                error_at_current("Can't have more than 255 parameters.");
            }
            consume(TOKEN_IDENTIFIER, "Expect parameter name.");
            parameters.push_back(previous());
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    StmtPtr body = statement();
    return Stmt::ptr(FunStmt(name, parameters, body));
}

StmtPtr Parser::let_declaration() {
    consume(TOKEN_IDENTIFIER, "Expected variable name.");
    Token name = previous();

    ExprPtr initializer = Expr::ptr(Literal(previous(), Nil{}));
    if (match(TOKEN_EQUAL)) initializer = expression();
    return Stmt::ptr(Let(name, initializer));
}

StmtPtr Parser::statement() {
    if (match(TOKEN_IF)) return if_statement();
    if (match(TOKEN_INDENT)) return block_statement();
    if (match(TOKEN_PRINT)) return print_statement();
    if (match(TOKEN_RETURN)) return return_statement();
    if (match(TOKEN_WHILE)) return while_statement();
    return expr_statement();
}

StmtPtr Parser::if_statement() {
    ExprPtr condition = expression();
    StmtPtr then_branch = statement();
    StmtPtr else_branch = nullptr;

    if (match(TOKEN_ELSE)) {
        else_branch = statement();
    }

    return Stmt::ptr(If(condition, then_branch, else_branch));
}

StmtPtr Parser::print_statement() {
    ExprPtr value = expression();
    return Stmt::ptr(Print(value));
}

StmtPtr Parser::return_statement() {
    Token debug = previous();
    ExprPtr value = expression();
    return Stmt::ptr(Return(debug, value));
}

StmtPtr Parser::block_statement() {
    std::vector<StmtPtr> block;
    while (!check(TOKEN_DEDENT) && !is_at_end()) {
        block.push_back(declaration());
    }
    if (!is_at_end()) {
        consume(TOKEN_DEDENT, "Expect 'Dedent' at end of block.");
    }
    return Stmt::ptr(Block(block));
}

StmtPtr Parser::while_statement() {
    ExprPtr condition = expression();
    StmtPtr body = statement();

    return Stmt::ptr(While(condition, body));
}

StmtPtr Parser::expr_statement() {
    return Stmt::ptr(ExprStmt(expression()));
}

ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = or_();

    if (match(TOKEN_EQUAL)) {
        if (expr->is_type(EXPR_VARIABLE)) {
            // Unsure about the recursion here.
            ExprPtr value = assignment();
            expr = Expr::ptr(Assign(expr->as<Variable>().name, value));
        }
    } else if (match({TOKEN_PLUS_EQUAL, TOKEN_MINUS_EQUAL, TOKEN_STAR_EQUAL, TOKEN_SLASH_EQUAL, TOKEN_MODULO_EQUAL})) {
        Token op = previous();
        if (expr->is_type(EXPR_VARIABLE)) {
            // Unsure about the recursion here.
            ExprPtr value = assignment();
            expr = Expr::ptr(CompoundAssign(expr->as<Variable>().name, op, value));
        }
    }

    return expr;
}

ExprPtr Parser::or_() {
    ExprPtr expr = and_();

    while (match(TOKEN_OR)) {
        Token op = previous();
        ExprPtr right = and_();
        expr = Expr::ptr(Logical(expr, op, right));
    }

    return expr;
}

ExprPtr Parser::and_() {
    ExprPtr expr = equality();

    while (match(TOKEN_AND)) {
        Token op = previous();
        ExprPtr right = equality();
        expr = Expr::ptr(Logical(expr, op, right));
    }

    return expr;
}

ExprPtr Parser::equality() {
    ExprPtr expr = comparison();

    while (match({TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL})) {
        Token op = previous();
        ExprPtr right = comparison();
        expr = Expr::ptr(Binary(expr, op, right));
    }

    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = term();

    while (match({TOKEN_LESS, TOKEN_LESS_EQUAL, TOKEN_GREATER, TOKEN_GREATER_EQUAL})) {
        Token op = previous();
        ExprPtr right = term();
        expr = Expr::ptr(Binary(expr, op, right));
    }

    return expr;
}

ExprPtr Parser::term() {
    ExprPtr expr = factor();

    while (match({TOKEN_MINUS, TOKEN_PLUS})) {
        Token op = previous();
        ExprPtr right = factor();
        expr = Expr::ptr(Binary(expr, op, right));
    }

    return expr;
}

ExprPtr Parser::factor() {
    ExprPtr expr = unary();

    while (match({TOKEN_STAR, TOKEN_SLASH, TOKEN_MODULO})) {
        Token op = previous();
        ExprPtr right = unary();
        expr = Expr::ptr(Binary(expr, op, right));
    }

    return expr;
}

ExprPtr Parser::unary() {
    if (match({TOKEN_BANG, TOKEN_MINUS})) {
        Token op = previous();
        ExprPtr right = call();
        return Expr::ptr(Unary(op, right));
    }

    return call();
}

ExprPtr Parser::call() {
    ExprPtr expr = primary();

    if (match(TOKEN_LEFT_PAREN)) {
        Token paren = previous();
        std::vector<ExprPtr> arguments;

        while (!check(TOKEN_RIGHT_PAREN)) {
            if (arguments.size() >= 255) {
                error_at_current("Can't have more than 255 arguments.");
            }
            arguments.push_back(expression());
            if (!match(TOKEN_COMMA)) break;
        }

        consume(TOKEN_RIGHT_PAREN, "Expect ')' after call.");
        if (!expr->is_type(EXPR_VARIABLE)) error_at_current("Invalid callee.");
        return Expr::ptr(Call(expr->as<Variable>().name, arguments));
    }

    return expr;
}

ExprPtr Parser::primary() {
    if (match(TOKEN_NIL)) {
        return Expr::ptr(Literal(previous(), Nil{}));
    }
    if (match(TOKEN_NUMBER)) {
        double number = std::stod(previous().lexeme);
        return Expr::ptr(Literal(previous(), number));
    }
    if (match(TOKEN_STRING)) {
        // Value given in compiler when string is interned
        return Expr::ptr(Literal(previous(), StringIndex{0}));
    }
    if (match({TOKEN_TRUE, TOKEN_FALSE})) {
        if (previous().type == TOKEN_TRUE) return Expr::ptr(Literal(previous(), true));
        else return Expr::ptr(Literal(previous(), false));
    }
    if (match(TOKEN_IDENTIFIER)) {
        return Expr::ptr(Variable(previous()));
    }
    if (match(TOKEN_LEFT_PAREN)) {
        ExprPtr expr = expression();
        consume(TOKEN_RIGHT_PAREN, "Expect ')', after expression.");
        return expr;
    }
    throw std::runtime_error("Expected expression.");
}

Token& Parser::previous() {
    return tokens[current - 1];
}

bool Parser::is_at_end() {
    return tokens[current].type == TOKEN_EOF;
}

bool Parser::match(std::vector<TokenType> types) {
    for (TokenType type : types ) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

bool Parser::check(TokenType type) {
    return tokens[current].type == type;
}

void Parser::advance() {
    while (true) {
        current++;
        if (tokens[current].type != TOKEN_ERROR) break;

        error_at_current(tokens[current].lexeme.c_str());
    }
}

void Parser::consume(TokenType type, const char* message) {
    if (!match({type})) {
        error_at_current(message);
    }
}

void Parser::error_at(Token& token, const char* message) {
    if (panic_mode) return;
    panic_mode = true;
    fprintf(stderr, "[line %d] Error", token.line);

    if (token.type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token.type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%s'", token.lexeme.c_str());
    }

    fprintf(stderr, ": %s\n", message);
    had_error = true;
}

void Parser::error(const char* message) {
    error_at(previous(), message);
}

void Parser::error_at_current(const char* message) {
    error_at(tokens[current], message);
}
