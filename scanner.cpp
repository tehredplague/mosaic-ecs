#include <iostream>
#include <sstream>

#include "debug.h"
#include "scanner.h"

Scanner::Scanner(std::string source) : source(source), indent_depth(0), start(0), current(0), line(1), column(0) {
    keywords = {
            { "else",   TOKEN_ELSE },
            { "false",  TOKEN_FALSE },
            { "fun",    TOKEN_FUN },
            { "if",     TOKEN_IF },
            { "nil",    TOKEN_NIL },
            { "print",  TOKEN_PRINT },
            { "return", TOKEN_RETURN },
            { "this",   TOKEN_THIS },
            { "true",   TOKEN_TRUE },
            { "let",    TOKEN_LET },
            { "while",  TOKEN_WHILE },
            { "World",  TOKEN_WORLD },
            { "Scene",  TOKEN_SCENE },
            { "Layer",  TOKEN_LAYER },
            { "Entity", TOKEN_ENTITY },
            { "comp",   TOKEN_COMP },
            { "sys",    TOKEN_SYS },
            { "Res",    TOKEN_RES },
            { "init",   TOKEN_INIT },
            { "run",    TOKEN_RUN },
            { "query", TOKEN_QUERY },
            { "with", TOKEN_WITH },
            { "without", TOKEN_WITHOUT }

    };
}

std::vector<Token> Scanner::scan_tokens() {
    while (!is_at_end()) {
        start = current;
        scan_token();
    }
    // Add EOF token.
    add_token(TOKEN_EOF);

#ifdef TOKEN_DEBUG
    std::cout << "==<Tokens>==" << std::endl;
    bool had_error = false;
    std::ostringstream error_log;
    for (auto token : tokens) {
        std::cout << token << std::endl;
        if (token.type == TOKEN_ERROR) {
            had_error = true;
            error_log << token << std::endl;
        }
    }
    if (had_error) std::cerr << error_log.str();
#endif

    return tokens;
}

void Scanner::scan_token() {
    char c = advance();

    switch (c) {
        case '[': add_token(TOKEN_LEFT_SQUARE); break;
        case ']': add_token(TOKEN_RIGHT_SQUARE); break;
        case '(': add_token(TOKEN_LEFT_PAREN); break;
        case ')': add_token(TOKEN_RIGHT_PAREN); break;
        case ':': add_token(TOKEN_COLON); break;
        case ',': add_token(TOKEN_COMMA); break;
        case '.': add_token(TOKEN_DOT); break;
        case '-':
            add_token( match('=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
            break;
        case '+':
            add_token( match('=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
            break;
        case '*':
            add_token( match('=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
            break;
        case '%':
            add_token( match('=') ? TOKEN_MODULO_EQUAL : TOKEN_MODULO);
            break;
        case '!':
            add_token( match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
            break;
        case '=':
            add_token( match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
            break;
        case '<':
            add_token( match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
            break;
        case '>':
            add_token( match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
            break;
        case '|':
            add_token( match('|') ? TOKEN_OR : TOKEN_PIPE);
            break;
        case '&':
            if (match('&')) { add_token(TOKEN_AND); break; }
        case '"': string(); break;
        case '/':
            if (match('/')) while (peek() != '\n' && !is_at_end()) advance();
            else if (match('*')) block_comment();
            else add_token( match('=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);
            break;
        case ' ':  case '\r': case '\t': case '{': case '}': case ';': break;
        case '\n': new_line(); break;
        default:
            if (is_alpha(c)) identifier();
            else if (is_digit(c)) number();
            else error_token("Unexpected character.");
            break;
    }
}

void Scanner::string() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (is_at_end()) return error_token("Unterminated string.");

    // The closing quote.
    advance();
    add_token(TOKEN_STRING);
}

void Scanner::block_comment() {
    while (!is_at_end()) {
        if (peek() == '*' && peek_next() == '/') {
            advance();
            advance();
            return;
        }
        advance();
    }
    error_token("Unterminated block comment");
}

void Scanner::new_line() {
    line++;
    column = 0;
    int whitespace = 0;
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':  case '\r': case '\t': case '{': case '}': case ';':
                advance();
                whitespace++;
                break;
            case '\n': return;
            default:
                if (whitespace > indent_depth) {
                    int indent = 0;
                    for (int dent : indents) {
                        indent += dent;
                    }
                    indents.push_back(whitespace - indent);
                    add_token(TOKEN_INDENT);
                } else if (whitespace < indent_depth) {
                    int difference = indent_depth - whitespace;
                    while (difference >= indents.back()) {
                        difference -= indents.back();
                        indents.pop_back();
                        add_token(TOKEN_DEDENT);
                        if (indents.empty()) break;
                    }
                }
                indent_depth = whitespace;
                return;
        }
    }
}

void Scanner::identifier() {
    while (is_alpha(peek()) || is_digit(peek())) advance();
    add_token(identifier_type());
}

TokenType Scanner::identifier_type() {
    std::string lexeme = source.substr(start, current - start);
    return (keywords.contains(lexeme)) ? keywords[lexeme] : TOKEN_IDENTIFIER;
}

void Scanner::number() {
    while (is_digit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && is_digit(peek_next())) {
        // Consume the ".".
        advance();

        while (is_digit(peek())) advance();
    }

    add_token(TOKEN_NUMBER);
}

char Scanner::advance() {
    column++;
    return source[current++];
}

char Scanner::peek() {
    return source[current];
}

char Scanner::peek_next() {
    if (is_at_end()) return '\0';
    return source[current + 1];
}

bool Scanner::match(char expected) {
    if (is_at_end()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

void Scanner::add_token(TokenType type) {
    Token token;
    token.type = type;
    token.line = line;
    if (type == TOKEN_INDENT || type == TOKEN_DEDENT  || type == TOKEN_EOF) {
        token.lexeme = "";
        token.column = 0;
    } else {
        token.lexeme = source.substr(start, current - start);
        token.column = column - (current - start) + 1;
    }
    tokens.push_back(token);
}

void Scanner::error_token(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme = message;
    token.line = line;
    token.column = column - (current - start) + 1;
    tokens.push_back(token);
}


bool Scanner::is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool Scanner::is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool Scanner::is_at_end() {
    return current >= source.length();
}
