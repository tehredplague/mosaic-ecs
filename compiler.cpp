#include <fstream>

#include "compiler.h"

Local::Local(std::string name, int depth, size_t stack_offset, size_t array_index, LocalType type) {
    this->name = name;
    this->resolution.depth = depth;
    this->resolution.stack_offset = stack_offset;
    this->resolution.array_index = array_index;
    this->resolution.type = type;
}

CompilerState::CompilerState(Compiler *compiler, std::vector<StmtPtr> &stmts) {
    this->stmts = stmts;
    current_stmt = stmts[0];
    previous_stmt = current_stmt;
    next = 0;
}

Compiler::Compiler(std::vector<StmtPtr> stmts) {
    if (stmts.empty()) exit(0);
    push_state(stmts);

    scope_depth = 0;

    functions.push_back(ObjFunction());
    current_function = 0;
}

void Compiler::compile() {
    while (!is_at_end()) {
        declaration();
    }
    emit_byte(OP_RETURN);
#ifdef BYTECODE_DEBUG
    for(int i = functions.size() - 1; i >= 0; i--) {
        current_function = i;
        Debugger debugger = Debugger(chunk(), functions, ffi, constants, strings);
        debugger.disassemble_chunk(functions[i].name.lexeme.empty() ? "Script" : functions[i].name.lexeme);
    }
#endif

    write();
}

void Compiler::declaration() {
    if (match(STMT_FUN)) fun_declaration();
    else if (match(STMT_LET)) let_declaration();
    else statement();
}

void Compiler::fun_declaration() {
    FunStmt& fun = previous()->as<FunStmt>();

    size_t previous_function = current_function;
    size_t index = new_function(ObjFunction(fun.name, fun.parameters.size()));
    current_function = index;

    push_locals();
    for (Token& param : fun.parameters) {
        new_variable(param);
        mark_initialized();
    }

    push_state({fun.body});
    while(!is_at_end()) {
        declaration();
    }
    pop_state();
    pop_locals();

    emit_return();
    current_function = previous_function;
}

void Compiler::let_declaration() {
    Let& let = previous()->as<Let>();
    new_variable(let.name);
    expression(*let.initializer);
    // Initialize variable.
    mark_initialized();
}

void Compiler::statement() {
    if (match(STMT_BLOCK)) block_statement();
    else if (match(STMT_EXPR)) {
        expression(*previous()->as<ExprStmt>().expr);
        emit_byte(OP_POP);
    }
    else if (match(STMT_IF)) if_statement();
    else if (match(STMT_PRINT)) print_statement();
    else if (match(STMT_RETURN)) {
        expression(*previous()->as<Return>().value);
        emit_byte(OP_RETURN);
    }
    else if (match(STMT_WHILE)) while_statement();
}

void Compiler::block_statement() {
    push_state(previous()->as<Block>().stmts);

    begin_scope();
    while (!is_at_end()) {
        declaration();
    }
    end_scope();

    pop_state();
}

void Compiler::if_statement() {
    If& if_stmt = previous()->as<If>();
    expression(*if_stmt.condition);

    int then_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);

    push_state({if_stmt.then_branch});

    //begin_scope();
    while (!is_at_end()) {
        declaration();
    }
    //end_scope();

    pop_state();
    int else_jump = emit_jump(OP_JUMP);

    patch_jump(then_jump);
    emit_byte(OP_POP);

    if (if_stmt.else_branch) {
        push_state({if_stmt.else_branch});

        //begin_scope();
        while (!is_at_end()) {
            declaration();
        }
        //end_scope();

        pop_state();
    }
    patch_jump(else_jump);
}

void Compiler::print_statement() {
    expression(*previous()->as<Print>().value);
    emit_byte(OP_PRINT);
}

void Compiler::while_statement() {
    While& while_stmt = previous()->as<While>();
    int loop_start = chunk().code.size();
    expression(*while_stmt.condition);

    int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    push_state({while_stmt.body});
    while (!is_at_end()) {
        declaration();
    }
    pop_state();
    emit_loop(loop_start);

    patch_jump(exit_jump);
    emit_byte(OP_POP);
}

void Compiler::expression(Expr& expr) {
    switch (expr.type) {
        case EXPR_ASSIGN: assign_expr(expr); break;
        case EXPR_COMPOUND_ASSIGN: compound_assign_expr(expr); break;
        case EXPR_BINARY: binary_expr(expr); break;
        case EXPR_CALL: call_expr(expr); break;
        case EXPR_LITERAL: literal_expr(expr); break;
        case EXPR_LOGICAL: logical_expr(expr); break;
        case EXPR_UNARY: unary_expr(expr); break;
        case EXPR_VARIABLE: variable_expr(expr); break;
    }
}

void Compiler::assign_expr(Expr &expr) {
    Assign& assign = expr.as<Assign>();
    uint8_t offset = resolve_variable(assign.name).resolution.stack_offset;

    expression(*assign.value);
    emit_bytes(OP_SET_LOCAL, offset);
}

void Compiler::compound_assign_expr(Expr &expr) {
    CompoundAssign& comp_assign = expr.as<CompoundAssign>();
    uint8_t offset = resolve_variable(comp_assign.name).resolution.stack_offset;

    expression(*comp_assign.value);
    switch (comp_assign.op.type) {
        case TOKEN_PLUS_EQUAL: emit_bytes(OP_ADD_ASSIGN, offset); break;
        case TOKEN_MINUS_EQUAL: emit_bytes(OP_SUBTRACT_ASSIGN, offset); break;
        case TOKEN_STAR_EQUAL: emit_bytes(OP_MULTIPLY_ASSIGN, offset); break;
        case TOKEN_SLASH_EQUAL: emit_bytes(OP_DIVIDE_ASSIGN, offset); break;
    }
}

void Compiler::binary_expr(Expr &expr) {
    expression(*expr.as<Binary>().left);
    expression(*expr.as<Binary>().right);
    switch (expr.as<Binary>().op.type) {
        case TOKEN_PLUS: emit_byte(OP_ADD); break;
        case TOKEN_PLUS_EQUAL: emit_byte(OP_ADD_ASSIGN); break;
        case TOKEN_MINUS: emit_byte(OP_SUBTRACT); break;
        case TOKEN_MINUS_EQUAL: emit_byte(OP_SUBTRACT_ASSIGN); break;
        case TOKEN_STAR: emit_byte(OP_MULTIPLY); break;
        case TOKEN_STAR_EQUAL: emit_byte(OP_MULTIPLY_ASSIGN); break;
        case TOKEN_SLASH: emit_byte(OP_DIVIDE); break;
        case TOKEN_SLASH_EQUAL: emit_byte(OP_DIVIDE_ASSIGN); break;
        case TOKEN_MODULO: emit_byte(OP_MODULO); break;
        case TOKEN_MODULO_EQUAL: emit_byte(OP_MODULO_ASSIGN); break;
        case TOKEN_EQUAL_EQUAL: emit_byte(OP_EQUAL); break;
        case TOKEN_BANG_EQUAL: emit_byte(OP_NOT_EQUAL); break;
        case TOKEN_GREATER: emit_byte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emit_byte(OP_GREATER_EQUAL); break;
        case TOKEN_LESS: emit_byte(OP_LESS); break;
        case TOKEN_LESS_EQUAL: emit_byte(OP_LESS_EQUAL); break;
        default:
            std::cerr << "Invalid binary operator '" << expr.as<Binary>().op.lexeme << "'." << std::endl;
            break;
    }
}

void Compiler::call_expr(Expr &expr) {
    Call& call = expr.as<Call>();

    Local local_function = resolve_function(call.callee);

    for (ExprPtr& arg : call.arguments) {
        expression(*arg);
    }
    switch (local_function.resolution.type) {
        case LOCAL_FUNCTION:
            emit_bytes(OP_CALL, local_function.resolution.array_index);
            break;
        case LOCAL_NATIVE_FUNCTION:
            emit_bytes(OP_CALL_NATIVE, local_function.resolution.array_index);
            break;
    }
}

void Compiler::literal_expr(Expr &expr) {
    Value& value = expr.as<Literal>().value;
    Token& token = expr.as<Literal>().token;
    switch (value_type(value)) {
        case VAL_STRING_INDEX: {
            std::string string = token.lexeme.substr(1, token.lexeme.length() - 2);
            auto result = string_intern.find(string);
            if (result != string_intern.end()) {
                emit_byte(OP_STRING);
                emit_byte(string_intern[string]);
                return;
            }
            StringIndex string_value = {strings.size()};
            emit_byte(OP_STRING);
            emit_byte((uint8_t)string_value.index);
            string_intern[string] = (uint8_t)string_value.index;
            strings.append(string);
            strings.push_back('\0');
        } break;
        default: emit_constant(value);
    }
}

void Compiler::logical_expr(Expr &expr) {
    Logical& logical = expr.as<Logical>();
    expression(*logical.left);

    switch (logical.op.type) {
        case TOKEN_OR: {
            int else_jump = emit_jump(OP_JUMP_IF_FALSE);
            int end_jump = emit_jump(OP_JUMP);

            patch_jump(else_jump);
            emit_byte(OP_POP);

            expression(*logical.right);
            patch_jump(end_jump);
            break;
        }
        case TOKEN_AND: {
            int end_jump = emit_jump(OP_JUMP_IF_FALSE);

            emit_byte(OP_POP);
            expression(*logical.right);

            patch_jump(end_jump);
            break;
        }
    }
}

void Compiler::unary_expr(Expr &expr) {
    expression(*expr.as<Unary>().right);
    switch (expr.as<Unary>().op.type) {
        case TOKEN_MINUS: emit_byte(OP_NEGATE); break;
        case TOKEN_BANG: emit_byte(OP_NOT); break;
    }
}

void Compiler::variable_expr(Expr &expr) {
    Variable& variable = expr.as<Variable>();
    Local& local = resolve_variable(variable.name);

    if (!local.resolution.fresh_function) {
        emit_bytes(OP_GET_LOCAL, local.resolution.stack_offset);
    } else {
        local.resolution.fresh_function = false;
    }
}

void Compiler::begin_scope() {
    scope_depth++;
}

void Compiler::end_scope() {
    scope_depth--;
    uint8_t pop_count = 0;
    for (int i  = locals().size() - 1; i >= 0; i--) {
        if (locals()[i].resolution.depth <= scope_depth) break;
        pop_count++;
        locals().pop_back();
    }
    if (pop_count) emit_bytes(OP_POP_N, pop_count);
}

void Compiler::new_variable(Token &name) {
    size_t stack_offset = locals().size();
    if (stack_offset != 0) {
        for (auto local = locals().rbegin(); local != locals().rend(); local++) {
            if (local->resolution.depth != -1 && local->resolution.depth < scope_depth) {
                break;
            }

            if (name.lexeme == local->name) {
                //error("Already a variable with this name in this scope.");
                std::cerr << "Already a variable with this name in this scope." << std::endl;
            }
        }

        if (locals().size() - 1 == UINT8_MAX) {
            //error("Too many local variables in function_index.");
            std::cerr << "Too many local variables in function_index." << std::endl;
            return;
        }
    // Depth of -1 marks uninitialized.
        locals().push_back(Local(name.lexeme, -1, stack_offset, -1, LOCAL_UNINITIALIZED));
    } else {
        locals_stack.push_back({Local(name.lexeme, -1, stack_offset, -1, LOCAL_UNINITIALIZED)});
    }
}

Local& Compiler::resolve_variable(Token &name) {
    for (int i = locals().size() - 1; i >= 0; --i) {
        Local& local = locals()[i];
        if (name.lexeme == local.name) {
            if (local.resolution.depth == -1) {
                //error("Can't read local variable in its own initializer.");
                std::cerr << "[line " << name.line << "]" << std::endl;
                std::cerr << "Can't read local variable in its own initializer." << std::endl;
            }
            if (local.resolution.type == LOCAL_UNINITIALIZED) {
                local.resolution.type = LOCAL_VARIABLE;
                local.resolution.fresh_function = false;
                if (local.resolution.array_index != -1)
                    std::cerr << "ERROR NOT LOCAL_VARIABLE" << std::endl;
            }
            return local;
        }
    }
    // TODO: Potential redundancy with resolve_function
    for (int i = 0; i < functions.size(); i++) {
        ObjFunction& func = functions[i];
        if (name.lexeme == func.name.lexeme) {
            locals().back().resolution.type = LOCAL_FUNCTION;
            locals().back().resolution.array_index = i;
            locals().back().resolution.fresh_function = true;
            // TODO: Potentially unecessary
            locals().back().resolution.stack_offset = locals().size() - 1;
            emit_constant(FunctionIndex(i, USER_FUNCTION));
            return locals().back();
        }
    }
    for (int i = ffi.native_functions.size() - 1; i >= 0; --i) {
        NativeFunction& native_func = ffi.native_functions[i];
        if (name.lexeme == native_func.name) {
            locals().back().resolution.type = LOCAL_NATIVE_FUNCTION;
            locals().back().resolution.array_index = i;
            locals().back().resolution.fresh_function = true;
            // TODO: Potentially unecessary
            locals().back().resolution.stack_offset = locals().size() - 1;
            emit_constant(FunctionIndex(i, NATIVE_FUNCTION));
            return locals().back();
        }
    }
    std::cerr << "[line " << name.line << "] " << "Undeclared variable: " << name.lexeme << std::endl;
    exit(-1);
}

size_t Compiler::new_function(ObjFunction func) {
    // TODO: Potential redundancy with resolve_function again
    for (ObjFunction& other_func : functions){
        if (func.name.lexeme == other_func.name.lexeme) {
            //error("Already a function with this.");
            std::cerr << "Already a function with this name ." << std::endl;
            return 0;
        }
    }
    functions.push_back(func);
    return functions.size() - 1;
}

Local Compiler::resolve_function(Token &name) {
    for (int i = functions.size() - 1; i >= 0; --i) {
        ObjFunction& func = functions[i];
        if (name.lexeme == func.name.lexeme) {
            return Local(name.lexeme, -1, 0, i, LOCAL_FUNCTION);
        }
    }
    for (int i = ffi.native_functions.size() - 1; i >= 0; --i) {
        NativeFunction& native_func = ffi.native_functions[i];
        if (name.lexeme == native_func.name) {
            return Local(name.lexeme, -1, 0, i, LOCAL_NATIVE_FUNCTION);
        }
    }
    for (int i = locals().size() - 1; i >= 0; --i) {
        Local& local = locals()[i];
        if (name.lexeme == local.name) {
            if (local.resolution.type == LOCAL_VARIABLE) break;
            return local;
            //{(uint8_t)i, false};
        }
    }
    std::cerr << "[line " << name.line << "] " << "Undeclared function: " << name.lexeme << std::endl;
    exit(-1);
}

void Compiler::mark_initialized() {
    //if (scope_depth == 0) return;
    locals().back().resolution.depth = scope_depth;
}

bool Compiler::match(StmtType type) {
    if (!current()->is_type(type)) return false;
    advance();
    return true;
}

void Compiler::advance() {
    next()++;
    previous() = current();
    if (!is_at_end()) current() = stmts()[next()];
}

bool Compiler::is_at_end() {
    return (size_t)next() >= stmts().size();
}

void Compiler::emit_constant(Value value) {
    if (value_type(value) == VAL_BOOL) {
        AS_BOOL(value) ? emit_byte(OP_TRUE) : emit_byte(OP_FALSE);
        return;
    }
    emit_bytes(OP_CONSTANT, make_constant(value));
}

uint8_t Compiler::make_constant(Value value) {
    if (constant_intern.contains(value)) {
        return constant_intern[value];
    }
    constants.push_back(value);
    int constant = constants.size() - 1;
    if (constant > UINT8_MAX) {
        //error("Too many constants in one chunk.");
        std::cerr << "Too many constants in one chunk." << std::endl;
        return 0;
    }

    constant_intern[value] = (uint8_t)constant;
    return (uint8_t)constant;
}

void Compiler::emit_byte(uint8_t byte) {
    chunk().code.push_back(byte);
    chunk().lines.push_back(0);
}

void Compiler::emit_bytes(uint8_t byte1, uint8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
}

void Compiler::emit_loop(int loop_start) {
    emit_byte(OP_LOOP);

    int offset = chunk().code.size() - loop_start + 2;
    if (offset > UINT16_MAX) {
        //error("Loop body too large.");
        std::cerr << "Loop body too large." << std::endl;
    }

    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
}

int Compiler::emit_jump(uint8_t instruction) {
    emit_byte(instruction);
    emit_byte(0xff);
    emit_byte(0xff);
    // Return location of jump.
    return chunk().code.size() - 2;
}

void Compiler::patch_jump(int offset) {
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = chunk().code.size() - offset - 2;

    if (jump > UINT16_MAX) {
        //error("Too much code to jump over.");
        std::cerr << "Too much code to jump over." << std::endl;
    }

    // Patch in the 16 bit jump in two uint_8.
    chunk().code[offset] = (jump >> 8) & 0xff;
    chunk().code[offset + 1] = jump & 0xff;
}

void Compiler::emit_return() {
    emit_byte(OP_NIL);
    emit_byte(OP_RETURN);
}

void Compiler::push_state(std::vector<StmtPtr> stmts) {
    state_stack.push_back(CompilerState(this, stmts));
}

void Compiler::pop_state() {
    state_stack.pop_back();
}

std::vector<StmtPtr>& Compiler::stmts() {
    return state_stack.back().stmts;
}

Chunk& Compiler::chunk() {
    return functions[current_function].chunk;
}

std::vector<Local>& Compiler::locals() {
    return locals_stack.back();
}

void Compiler::push_locals() {
    locals_stack.resize(locals_stack.size() + 1);
}

void Compiler::pop_locals() {
    locals_stack.pop_back();
}


StmtPtr& Compiler::current() {
    return state_stack.back().current_stmt;
}

StmtPtr& Compiler::previous() {
    return state_stack.back().previous_stmt;
}

int& Compiler::next() {
    return state_stack.back().next;
}

void Compiler::write() {
    std::ofstream out("bytecode.dat", std::ios::binary);
    if (out.is_open()) {
        // Function Count
        size_t functions_size = functions.size();
        out.write(reinterpret_cast<char*>(&functions_size), sizeof(size_t));
        for (size_t i = 0; i < functions_size; i++) {
            // Function Token
            size_t name_size = functions[i].name.lexeme.size();
            out.write(reinterpret_cast<char*>(&name_size), sizeof(size_t));
            out.write(reinterpret_cast<char*>(functions[i].name.lexeme.data()), name_size * sizeof(char));
            // Function Arity
            out.write(reinterpret_cast<char*>(&functions[i].arity), 1 * sizeof(int));
            // CHUNK
            // Bytecode:
            size_t bytecode_size = functions[i].chunk.code.size();
            out.write(reinterpret_cast<char*>(&bytecode_size), sizeof(size_t));
            out.write(reinterpret_cast<char*>(functions[i].chunk.code.data()), bytecode_size * sizeof(uint8_t));
            // Lines:
            size_t lines_size = functions[i].chunk.lines.size();
            out.write(reinterpret_cast<char*>(&lines_size), sizeof(size_t));
            out.write(reinterpret_cast<char*>(functions[i].chunk.lines.data()), bytecode_size * sizeof(int));
        }
        // Constants:
        size_t constants_size = constants.size();
        out.write(reinterpret_cast<char*>(&constants_size), sizeof(size_t));
        out.write(reinterpret_cast<char*>(constants.data()), constants_size * sizeof(Value));
        // StringExpr Constants:
        size_t str_constants_size = strings.size();
        out.write(reinterpret_cast<char*>(&str_constants_size), sizeof(size_t));
        out.write(reinterpret_cast<char*>(strings.data()), str_constants_size * sizeof(char));

        out.close();
    }
}
