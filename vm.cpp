#include <fstream>

#include "debug.h"
#include "vm.h"

VM::VM() {
    frames.push_back(CallFrame(0, 0));
    read();
}


RuntimeResult VM::run() {
#ifdef VM_DEBUG
    std::cout << "==<VM>==";
#endif
#define BINARY_OP(op) \
    while (true) {              \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtime_error("Operands must be numbers."); \
        return RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(a op b);   \
      break; \
    }
#define COMPOUND_BINARY_OP(op) \
    while (true) {              \
      Value& value = stack()[read_byte()]; \
      if (!IS_NUMBER(value) || !IS_NUMBER(peek(0))) { \
        runtime_error("Operands must be numbers."); \
        return RUNTIME_ERROR; \
      } \
      AS_NUMBER(value) op AS_NUMBER(peek(0)); \
      break; \
    }

    while (true) {
#ifdef VM_DEBUG
        printf("          ");
        for (Value& value : value_stack) {
            std::cout << "[ ";
            print_value(value, strings, functions, ffi);
            std::cout << " ]";
        }
        std::cout << std::endl;
        Debugger debugger(functions[frame().function_index].chunk, functions, ffi, constants, strings);
        debugger.disassemble_instruction(frame().ip);
#endif
        switch (read_byte()) {
            case OP_CONSTANT: push(read_constant()); break;
            case OP_NIL: push(Nil{}); break;
            case OP_TRUE: push(true); break;
            case OP_FALSE: push(false); break;
            case OP_STRING: string(); break;
            case OP_POP: pop(); break;
            case OP_POP_N:
                for (uint8_t i = read_byte(); i >= 1; i--) pop();
                break;
            case OP_GET_LOCAL:
                push(stack()[read_byte()]);
                break;
            case OP_SET_LOCAL:
                stack()[read_byte()] = peek(0);
                break;
            case OP_ADD_ASSIGN: COMPOUND_BINARY_OP(+=); break;
            case OP_SUBTRACT_ASSIGN: COMPOUND_BINARY_OP(-=); break;
            case OP_MULTIPLY_ASSIGN: COMPOUND_BINARY_OP(*=); break;
            case OP_DIVIDE_ASSIGN: COMPOUND_BINARY_OP(/=); break;
            case OP_MODULO_ASSIGN: {
                double& value = AS_NUMBER(stack()[read_byte()]);
                long temp_value = value;
                temp_value %= (long)AS_NUMBER(peek(0));
                value = temp_value;
                break;
            }
            case OP_ADD:
                if (IS_STRING_INDEX(peek(0)) && IS_STRING_INDEX(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(a + b);
                } else {
                    runtime_error("Operands must be two numbers or two strings.");
                    return RUNTIME_ERROR;
                }
                break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE: BINARY_OP(/); break;
            case OP_MODULO: {
                long b = AS_NUMBER(pop());
                long a = AS_NUMBER(pop());
                push((double)(a % b));
                break;
            }
            case OP_LESS: BINARY_OP(<); break;
            case OP_LESS_EQUAL: BINARY_OP(<=); break;
            case OP_GREATER: BINARY_OP(>); break;
            case OP_GREATER_EQUAL: BINARY_OP(>=); break;
            case OP_NOT:
                push(is_falsey(pop()));
                break;
            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    //runtimeError("Operand must be a number.");
                    return RUNTIME_ERROR;
                }
                push(-AS_NUMBER(pop()));
                break;
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(values_equal(a, b));
                break;
            }
            case OP_NOT_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(!values_equal(a, b));
                break;
            }
            case OP_PRINT: print_value(pop(), strings, functions, ffi); std::cout << std::endl; break;
            case OP_JUMP: {
                uint16_t offset = read_short();
                frame().ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = read_short();
                // Potential error old vs new
                if (is_falsey(peek(0))) frame().ip += offset;
                //if (is_falsey(pop())) ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = read_short();
                frame().ip -= offset;
                break;
            }
            case OP_CALL: {
                uint8_t function_index = read_byte();
                call(function_index);
                break;
            }
            case OP_CALL_NATIVE: {
                uint8_t function_index = read_byte();
                call_native(function_index);
                break;
            }
            case OP_RETURN: {
                Value result = pop();
                size_t slots = frame().slots;
                frames.pop_back();
                if (frames.empty()) {
                    pop();
                    return RUNTIME_OK;
                }

                //vm.stackTop = frame->slots;
                while (value_stack.size() > slots) {
                    pop();
                }
                push(result);
                //frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            default: return RUNTIME_ERROR;
        }
    }
#undef BINARY_OP
#undef COMPOUND_BINARY_OP
}

void VM::string() {
    size_t index = read_byte();
    auto result = string_intern.find(&strings[index]);

    if (result != string_intern.end()) {
        push(StringIndex{result->second});
        return;
    }
    string_intern[&strings[index]] = index;
    push(StringIndex{index});
}

bool VM::call(int function_index) {
    ObjFunction& function = functions[function_index];
    frames.push_back(CallFrame(function_index, value_stack.size() - functions[function_index].arity));
    return true;
}
bool VM::call_native(int function_index) {
    NativeFunction& native_fn = ffi.native_functions[function_index];
    NativeFn native = native_fn.native_fn;
    Value result = native(native_fn.arity, stack() - native_fn.arity);
    for (int i = 0; i < native_fn.arity; i++) {
        value_stack.pop_back();
    }
    push(result);
    return true;
}

void VM::concatenate() {
    const char* b = &strings[AS_STRING_INDEX(pop()).index];
    std::string a_b = &strings[AS_STRING_INDEX(pop()).index];
    a_b.append(b);

    auto result = string_intern.find(a_b);
    if (result != string_intern.end()) {
        push(StringIndex{result->second});
        return;
    }

    uint8_t index = strings.size();
    string_intern[a_b] = index;
    strings.append(a_b);
    strings.push_back('\0');
    push(StringIndex{index});
}

bool VM::is_falsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

uint8_t VM::read_byte() {
    return functions[frame().function_index].chunk.code[frame().ip++];
}

uint16_t VM::read_short() {
    return (uint16_t)(read_byte() << 8) | read_byte();
}

Value VM::read_constant() {
    return constants[read_byte()];
}

void VM::push(Value value) {
    value_stack.push_back(value);
}

Value VM::pop() {
    Value value = value_stack.back();
    value_stack.pop_back();
    return value;
}

Value VM::peek(int distance) {
    return value_stack[value_stack.size() - 1 - distance];
}

Value* VM::stack() {
    return value_stack.data() + frame().slots;
}

CallFrame& VM::frame() {
    return frames.back();
}

void VM::runtime_error(const char* message) {
    std::cerr << message << std::endl;
}

void VM::read() {
    std::ifstream in("bytecode.dat", std::ios::binary);

    if (in.is_open()) {
        // Function Count
        size_t functions_size;
        in.read(reinterpret_cast<char*>(&functions_size), sizeof(size_t));
        functions.resize(functions_size);
        for (size_t i = 0; i < functions_size; i++) {
            // Function Name
            size_t name_size;
            in.read(reinterpret_cast<char*>(&name_size), sizeof(size_t));
            functions[i].name.lexeme.resize(name_size);
            in.read(reinterpret_cast<char*>(functions[i].name.lexeme.data()), name_size * sizeof(char));
            // Function Arity
            in.read(reinterpret_cast<char*>(&functions[i].arity), 1 * sizeof(int));
            // Bytecode:
            size_t bytecode_size;
            in.read(reinterpret_cast<char*>(&bytecode_size), sizeof(size_t));
            functions[i].chunk.code.resize(bytecode_size);
            in.read(reinterpret_cast<char*>(functions[i].chunk.code.data()), bytecode_size * sizeof(uint8_t));
            // Lines:
            size_t lines_size;
            in.read(reinterpret_cast<char*>(&lines_size), sizeof(size_t));
            functions[i].chunk.lines.resize(lines_size);
            in.read(reinterpret_cast<char*>(functions[i].chunk.lines.data()), bytecode_size * sizeof(int));
        }
        // Constants
        size_t constants_size;
        in.read(reinterpret_cast<char*>(&constants_size), sizeof(size_t));
        constants.resize(constants_size);
        in.read(reinterpret_cast<char*>(constants.data()), constants_size * sizeof(Value));
        // Constants
        size_t str_constants_size;
        in.read(reinterpret_cast<char*>(&str_constants_size), sizeof(size_t));
        strings.resize(str_constants_size);
        in.read(reinterpret_cast<char*>(strings.data()), str_constants_size * sizeof(char));

        in.close();
    }
}