#ifndef MOSAIC_ECS_DEBUG_H
#define MOSAIC_ECS_DEBUG_H

#define TOKEN_DEBUG
#define STMT_DEBUG
#define BYTECODE_DEBUG
#define VM_DEBUG

#include <vector>

#include "value.h"

class Chunk;
class ObjFunction;

class Debugger {
public:
    Debugger(Chunk& chunk, std::vector<ObjFunction>& functions, class FFI& ffi, std::vector<Value>& constants, std::string& strings);
    void disassemble_chunk(std::string name);
    int disassemble_instruction(int offset);
private:
    int constant_instruction(const char* name, int offset);
    int string_instruction(const char* name, int offset);
    int function_instruction(const char* name, int offset);
    int simple_instruction(const char* name, int offset);
    int byte_instruction(const char* name, int offset);
    int jump_instruction(const char* name, int sign, int offset);

    Chunk& chunk;
    std::vector<ObjFunction>& functions;
    FFI& ffi;
    std::vector<Value>& constants;
    std::string& strings;
};

#endif
