#include "value.h"
#include "ffi.h"

ValType value_type(const Value& value) {
    if (IS_BOOL(value)) return VAL_BOOL;
    if (IS_FUNCTION_INDEX(value)) return VAL_FUNCTION_INDEX;
    if (IS_NIL(value)) return VAL_NIL;
    if (IS_NUMBER(value)) return VAL_NUMBER;
    if (IS_STRING_INDEX(value)) return VAL_STRING_INDEX;
    std::cerr << "VALUE HAS BAD TYPE ]" << std::endl;
}

bool values_equal(Value& a, Value& b) {
    ValType a_type = value_type(a);
    ValType b_type = value_type(b);
    if (a_type != b_type) return false;
    switch (a_type) {
        case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
        case VAL_FUNCTION_INDEX: return AS_FUNCTION_INDEX(a) == AS_FUNCTION_INDEX(b);
        case VAL_NIL: return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_STRING_INDEX: return AS_STRING_INDEX(a) == AS_STRING_INDEX(b);
        default:                return false; // Unreachable.
    }
}

void print_value(const Value& value, std::string& strings, std::vector<ObjFunction>& functions, FFI& ffi) {
    switch (value_type(value)) {
        case VAL_FUNCTION_INDEX: {
            const FunctionIndex& fn_index = AS_FUNCTION_INDEX(value);
            if (fn_index.user_index != -1) {
                std::cout << "<fn " << functions[AS_FUNCTION_INDEX(value).user_index].name.lexeme << ">"; break;
            } else {
                std::cout << "<native " << ffi.native_functions[AS_FUNCTION_INDEX(value).native_index].name << ">"; break;
            }
        }
        case VAL_NUMBER: std::cout << AS_NUMBER(value); break;
        case VAL_STRING_INDEX: {
            std::cout << &strings[AS_STRING_INDEX(value).index];
            break;
        }
        case VAL_BOOL: std::cout << (AS_BOOL(value) ? "true" : "false"); break;
        case VAL_NIL: std::cout << "nil"; break;
    }
}
