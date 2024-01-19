#ifndef MOSAIC_ECS_VM_H
#define MOSAIC_ECS_VM_H

#include <unordered_map>

#include "ffi.h"

enum RuntimeResult {
    RUNTIME_OK,
    RUNTIME_ERROR,
};

struct CallFrame {
    CallFrame(size_t function_index, size_t slots) {
       this->function_index = function_index;
       this->slots = slots;
    }
    size_t function_index;
    size_t ip = 0;
    size_t slots;
};

class VM {
public:
    VM();
    RuntimeResult run();
private:
    uint8_t read_byte();
    uint16_t read_short();
    Value read_constant();
    void concatenate();
    void string();
    bool call(int function_index);
    bool call_native(int function_index);
    bool is_falsey(Value value);
    void push(Value value);
    Value pop();
    Value peek(int distance);
    Value* stack();
    CallFrame& frame();
    void runtime_error(const char* message);
    void read();

    std::vector<CallFrame> frames;
    std::vector<Value> value_stack;
    std::vector<Value> constants;
    std::string strings;
    std::unordered_map<std::string, uint8_t> string_intern;
    std::vector<ObjFunction> functions;

    FFI ffi;
};

#endif
