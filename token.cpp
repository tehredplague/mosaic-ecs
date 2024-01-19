#include "debug.h"

#ifdef TOKEN_DEBUG
#include <iostream>
#include <unordered_map>

#include "token.h"

std::unordered_map<TokenType, std::string> TokenTypeStrings = {
        {TOKEN_LEFT_SQUARE, "TOKEN_LEFT_SQUARE"},
        {TOKEN_RIGHT_SQUARE, "TOKEN_RIGHT_SQUARE"},
        {TOKEN_LEFT_PAREN, "TOKEN_LEFT_PAREN"},
        {TOKEN_RIGHT_PAREN, "TOKEN_RIGHT_PAREN"},
        {TOKEN_INDENT, "TOKEN_INDENT"},
        {TOKEN_DEDENT, "TOKEN_DEDENT"},
        {TOKEN_COLON, "TOKEN_COLON"},
        {TOKEN_COMMA, "TOKEN_COMMA"},
        {TOKEN_DOT, "TOKEN_DOT"},
        {TOKEN_MINUS, "TOKEN_MINUS"},
        {TOKEN_MINUS_EQUAL, "TOKEN_MINUS_EQUAL"},
        {TOKEN_PLUS, "TOKEN_PLUS"},
        {TOKEN_PLUS_EQUAL, "TOKEN_PLUS_EQUAL"},
        {TOKEN_SLASH, "TOKEN_SLASH"},
        {TOKEN_SLASH_EQUAL, "TOKEN_SLASH_EQUAL"},
        {TOKEN_STAR, "TOKEN_STAR"},
        {TOKEN_STAR_EQUAL, "TOKEN_STAR_EQUAL"},
        {TOKEN_MODULO, "TOKEN_MODULO"},
        {TOKEN_MODULO_EQUAL, "TOKEN_MODULO_EQUAL"},
        {TOKEN_BANG, "TOKEN_BANG"},
        {TOKEN_BANG_EQUAL, "TOKEN_BANG_EQUAL"},
        {TOKEN_EQUAL, "TOKEN_EQUAL"},
        {TOKEN_EQUAL_EQUAL, "TOKEN_EQUAL_EQUAL"},
        {TOKEN_GREATER, "TOKEN_GREATER"},
        {TOKEN_GREATER_EQUAL, "TOKEN_GREATER_EQUAL"},
        {TOKEN_LESS, "TOKEN_LESS"},
        {TOKEN_LESS_EQUAL, "TOKEN_LESS_EQUAL"},
        {TOKEN_PIPE, "TOKEN_PIPE"},
        {TOKEN_OR, "TOKEN_OR"},
        {TOKEN_AND, "TOKEN_AND"},
        {TOKEN_IDENTIFIER, "TOKEN_IDENTIFIER"},
        {TOKEN_STRING, "TOKEN_STRING"},
        {TOKEN_NUMBER, "TOKEN_NUMBER"},
        {TOKEN_ELSE,   "TOKEN_ELSE"},
        {TOKEN_FALSE,  "TOKEN_FALSE"},
        {TOKEN_FUN,    "TOKEN_FUN"},
        {TOKEN_IF,     "TOKEN_IF"},
        {TOKEN_NIL,    "TOKEN_NIL"},
        {TOKEN_PRINT,  "TOKEN_PRINT"},
        {TOKEN_RETURN, "TOKEN_RETURN"},
        {TOKEN_THIS,   "TOKEN_THIS"},
        {TOKEN_TRUE,   "TOKEN_TRUE"},
        {TOKEN_LET,    "TOKEN_LET"},
        {TOKEN_WHILE,  "TOKEN_WHILE"},

        {TOKEN_WORLD,  "TOKEN_WORLD"},
        {TOKEN_SCENE,  "TOKEN_SCENE"},
        {TOKEN_LAYER,  "TOKEN_LAYER"},
        {TOKEN_ENTITY, "TOKEN_ENTITY"},
        {TOKEN_COMP,   "TOKEN_COMP"},
        {TOKEN_SYS,    "TOKEN_SYS"},
        {TOKEN_RES,    "TOKEN_RES"},
        {TOKEN_INIT,   "TOKEN_INIT"},
        {TOKEN_RUN, "TOKEN_INIT"},
        {TOKEN_QUERY, "TOKEN_QUERY"},
        {TOKEN_WITH, "TOKEN_WITH"},
        {TOKEN_WITHOUT, "TOKEN_WITHOUT"},

        {TOKEN_ERROR, "TOKEN_ERROR"},
        {TOKEN_EOF, "TOKEN_EOF"}
};

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << token.line << ":" << token.column << " " << TokenTypeStrings[token.type] << " " << token.lexeme;
    return os;
}
#endif
