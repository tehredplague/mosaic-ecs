#ifndef MOSAIC_ECS_FFI_H
#define MOSAIC_ECS_FFI_H

#include <string>
#include "value.h"

using NativeFn = Value (*)(int arg_count, Value* args);

struct NativeFunction {
    NativeFunction(std::string name, NativeFn function, int arity) {
        this->native_fn = function;
        this->arity = arity;
        this->name = name;
    }
    NativeFn native_fn;
    int arity;
    std::string name;
};

static Value clock_native(int argCount, Value* args) {
    return (double)clock() / CLOCKS_PER_SEC;
}

class FFI {
public:
    FFI() {
        define_function("clock", clock_native, 0);
    }
    void define_function(std::string name, NativeFn function, int arity) {
        native_functions.push_back(NativeFunction(name, function, arity));
    }
    std::vector<NativeFunction> native_functions;
};

#endif
