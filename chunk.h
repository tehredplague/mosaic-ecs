#ifndef MOSAIC_ECS_CHUNK_H
#define MOSAIC_ECS_CHUNK_H

#include <stdint.h>
#include <string>
#include <vector>

#include "token.h"

enum OpCode {
    OP_CONSTANT,
    OP_STRING,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_POP_N,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_ADD,
    OP_ADD_ASSIGN,
    OP_SUBTRACT,
    OP_SUBTRACT_ASSIGN,
    OP_MULTIPLY,
    OP_MULTIPLY_ASSIGN,
    OP_DIVIDE,
    OP_DIVIDE_ASSIGN,
    OP_MODULO,
    OP_MODULO_ASSIGN,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_CALL_NATIVE,
    OP_RETURN,
};

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<int> lines;
};

struct ObjFunction {
    ObjFunction() {};
    ObjFunction(Token name, int arity) {
        this->name = name;
        this->arity = arity;
    }
    int arity = 0;
    Chunk chunk;
    Token name;
};

#endif
