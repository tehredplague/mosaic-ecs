#ifndef MOSAIC_ECS_PARSER_H
#define MOSAIC_ECS_PARSER_H

#include <vector>

#include "stmt.h"
#include "token.h"

class Parser {
public:
    Parser(std::vector<Token> tokens);
    std::vector<StmtPtr> parse();
private:
    StmtPtr declaration();
    StmtPtr fun_declaration();
    StmtPtr let_declaration();
    StmtPtr statement();
    StmtPtr if_statement();
    StmtPtr block_statement();
    StmtPtr print_statement();
    StmtPtr return_statement();
    StmtPtr while_statement();
    StmtPtr expr_statement();
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr or_();
    ExprPtr and_();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr call();
    ExprPtr primary();
    Token& previous();
    bool is_at_end();
    bool match(std::vector<TokenType> types);
    bool match(TokenType type);
    bool check(TokenType type);
    void advance();
    void consume(TokenType type, const char* message);
    void error(const char* message);
    void error_at(Token& token, const char* message);
    void error_at_current(const char* message);

    std::vector<StmtPtr> stmts;
    std::vector<Token> tokens;
    int current;
    int scope_depth;

    bool panic_mode;
    bool had_error;
};

#endif
