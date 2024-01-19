#ifndef MOSAIC_ECS_VALUE_H
#define MOSAIC_ECS_VALUE_H

#include <variant>
#include <string>
#include <iostream>

#include "chunk.h"

enum FunctionType {
    USER_FUNCTION,
    NATIVE_FUNCTION,
};

struct FunctionIndex {
    FunctionIndex(int index, FunctionType type) {
        switch (type) {
            case USER_FUNCTION:
                this->user_index = index;
                this->native_index = -1;
                break;
            case NATIVE_FUNCTION:
                this->native_index = index;
                this->user_index = -1;
                break;
        }
    }

    int user_index;
    int native_index;
    bool operator==(const FunctionIndex& other) const {
        if (user_index != -1) return user_index == other.user_index;
        if (native_index != -1) return native_index == other.native_index;
        return false;
    }
};

struct Nil {
    bool operator==(const Nil&) const { return true; }
};

struct StringIndex {
    size_t index;
    bool operator==(const StringIndex& other) const { return index == other.index; }
};

typedef std::variant<bool, double, FunctionIndex, Nil, StringIndex> Value;
#define IS_BOOL(value) std::holds_alternative<bool>(value)
#define IS_FUNCTION_INDEX(value) std::holds_alternative<FunctionIndex>(value)
#define IS_NUMBER(value) std::holds_alternative<double>(value)
#define IS_NIL(value) std::holds_alternative<Nil>(value)
#define IS_STRING_INDEX(value) std::holds_alternative<StringIndex>(value)

#define AS_BOOL(value) std::get<bool>(value)
#define AS_FUNCTION_INDEX(value) std::get<FunctionIndex>(value)
#define AS_NUMBER(value) std::get<double>(value)
#define AS_STRING_INDEX(value) std::get<StringIndex>(value)

enum ValType {
    VAL_BOOL,
    VAL_FUNCTION_INDEX,
    VAL_NUMBER,
    VAL_NIL,
    VAL_STRING_INDEX,
};

ValType value_type(const Value& value);
bool values_equal(Value& a, Value& b);
void print_value(const Value& value, std::string& strings, std::vector<ObjFunction>& functions, class FFI& ffi );

struct ValueHash {
    template <class T>
    static void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

    std::size_t operator()(const Value& val) const {
        std::size_t seed = 0;
        std::visit([&](auto&& arg){
            using T = std::decay_t<decltype(arg)>;
            if constexpr(!std::is_same_v<T, Nil>) {
                if constexpr(std::is_same_v<T, FunctionIndex>) {
                    hash_combine(seed, arg.user_index);
                } else if constexpr (std::is_same_v<T, StringIndex>) {
                    hash_combine(seed, arg.index);
                } else {
                    hash_combine(seed, arg);
                }
            }
        }, val);
        return seed;
    }
};

#endif
