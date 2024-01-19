#include <cstdint>

#include "chunk.h"
#include "debug.h"
#include "ffi.h"

Debugger::Debugger(Chunk& chunk,
        std::vector<ObjFunction>& functions,
        FFI& ffi,
        std::vector<Value>& constants,
        std::string& strings)
        : chunk(chunk), functions(functions), ffi(ffi), constants(constants), strings(strings) {}

void Debugger::disassemble_chunk(std::string name) {
    std::cout << "==<" << name << ">==" << std::endl;

    for (int offset = 0; offset < chunk.code.size();) {
        offset = disassemble_instruction(offset);
    }
}

int Debugger::constant_instruction(const char* name, int offset) {
    uint8_t constant = chunk.code[offset + 1];
    printf("%-16s %4d '", name, constant);
    print_value(constants[constant], strings, functions, ffi);
    std::cout << '\'' << std::endl;
    return offset + 2;
}

int Debugger::string_instruction(const char* name, int offset) {
    uint8_t constant = chunk.code[offset + 1];
    printf("%-16s %4d '", name, constant);
    std::cout << &strings[constant] << '\'' << std::endl;
    return offset + 2;
}

int Debugger::function_instruction(const char* name, int offset) {
    uint8_t constant = chunk.code[offset + 1];
    printf("%-16s %4d '", name, constant);
    std::cout << functions[constant].name.lexeme << '\'' << std::endl;
    return offset + 2;
}

int Debugger::simple_instruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

int Debugger::byte_instruction(const char* name, int offset) {
    // No support for local variable identifier in debug
    uint8_t slot = chunk.code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

int Debugger::jump_instruction(const char* name, int sign, int offset) {
    uint16_t jump = (uint16_t)(chunk.code[offset + 1] << 8);
    jump |= chunk.code[offset + 2];
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

int Debugger::disassemble_instruction(int offset) {
    printf("%04d ", offset);
    if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk.lines[offset]);
    }

    uint8_t instruction = chunk.code[offset];
    switch (instruction) {
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", offset);
        case OP_STRING:
            return string_instruction("OP_STRING", offset);
        case OP_NIL:
            return simple_instruction("OP_NIL", offset);
        case OP_TRUE:
            return simple_instruction("OP_TRUE", offset);
        case OP_FALSE:
            return simple_instruction("OP_FALSE", offset);
        case OP_POP:
            return simple_instruction("OP_POP", offset);
        case OP_POP_N:
            return byte_instruction("OP_POP_N", offset);
        case OP_GET_LOCAL:
            return byte_instruction("OP_GET_LOCAL", offset);
        case OP_SET_LOCAL:
            return byte_instruction("OP_SET_LOCAL", offset);
        case OP_GET_GLOBAL:
            return constant_instruction("OP_GET_GLOBAL", offset);
        case OP_DEFINE_GLOBAL:
            return constant_instruction("OP_DEFINE_GLOBAL", offset);
        case OP_SET_GLOBAL:
            return constant_instruction("OP_SET_GLOBAL", offset);
        case OP_EQUAL:
            return simple_instruction("OP_EQUAL", offset);
        case OP_NOT_EQUAL:
            return simple_instruction("OP_NOT_EQUAL", offset);
        case OP_GREATER:
            return simple_instruction("OP_GREATER", offset);
        case OP_GREATER_EQUAL:
            return simple_instruction("OP_GREATER_EQUAL", offset);
        case OP_LESS:
            return simple_instruction("OP_LESS", offset);
        case OP_LESS_EQUAL:
            return simple_instruction("OP_LESS_EQUAL", offset);
        case OP_ADD:
            return simple_instruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simple_instruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset);
        case OP_MODULO:
            return simple_instruction("OP_MODULO", offset);
        case OP_ADD_ASSIGN:
            return byte_instruction("OP_ADD_ASSIGN", offset);
        case OP_SUBTRACT_ASSIGN:
            return byte_instruction("OP_SUBTRACT_ASSIGN", offset);
        case OP_MULTIPLY_ASSIGN:
            return byte_instruction("OP_MULTIPLY_ASSIGN", offset);
        case OP_DIVIDE_ASSIGN:
            return byte_instruction("OP_DIVIDE_ASSIGN", offset);
        case OP_MODULO_ASSIGN:
            return byte_instruction("OP_MODULO_ASSIGN", offset);
        case OP_NOT:
            return simple_instruction("OP_NOT", offset);
        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset);
        case OP_PRINT:
            return simple_instruction("OP_PRINT", offset);
        case OP_JUMP:
            return jump_instruction("OP_JUMP", 1, offset);
        case OP_JUMP_IF_FALSE:
            return jump_instruction("OP_JUMP_IF_FALSE", 1, offset);
        case OP_LOOP:
            return jump_instruction("OP_LOOP", -1, offset);
        case OP_CALL:
            return function_instruction("OP_CALL", offset);
        case OP_CALL_NATIVE:
            return byte_instruction("OP_CALL_NATIVE", offset);
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
