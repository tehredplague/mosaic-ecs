#ifndef MOSAIC_ECS_SCANNER_H
#define MOSAIC_ECS_SCANNER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "token.h"

class Scanner {
public:
    Scanner(std::string source);
    std::vector<Token> scan_tokens();
private:
    void scan_token();
    void string();
    void block_comment();
    void new_line();
    void number();
    void identifier();
    TokenType identifier_type();
    char advance();
    char peek();
    char peek_next();
    bool match(char expected);
    void add_token(TokenType type);
    void error_token(const char* message);
    bool is_alpha(char c);
    bool is_digit(char c);
    bool is_at_end();

    std::vector<Token> tokens;
    std::string source;
    int indent_depth;
    std::vector<int> indents;
    int start;
    int current;
    int line;
    int column;
    std::unordered_map<std::string, TokenType> keywords;
};

#endif
