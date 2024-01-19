#ifndef MOSAIC_ECS_COMPILER_H
#define MOSAIC_ECS_COMPILER_H

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "chunk.h"
#include "debug.h"
#include "ffi.h"
#include "stmt.h"

enum LocalType {
    LOCAL_UNINITIALIZED,
    LOCAL_VARIABLE,
    LOCAL_FUNCTION,
    LOCAL_NATIVE_FUNCTION,
};

struct LocalResolution {
    LocalType type;
    uint8_t stack_offset;
    int array_index;
    int depth;

    bool fresh_function;
};

struct Local {
    Local(std::string name, int depth, size_t stack_offset, size_t array_index, LocalType type);
    std::string name;
    LocalResolution resolution;
};

class Compiler;

struct CompilerState {
    CompilerState(Compiler* compiler, std::vector<StmtPtr>& stmts);
    std::vector<StmtPtr> stmts;
    StmtPtr current_stmt;
    StmtPtr previous_stmt;
    int next;
};

enum CompilerResult {
    COMPILER_RESULT_ERROR,
    COMPILER_RESULT_OK,
};

class Compiler {
public:
    Compiler(std::vector<StmtPtr> stmts);
    void compile();
    friend struct CompilerState;
    friend class Debugger;
private:
    void declaration();
    void fun_declaration();
    void let_declaration();
    void statement();
    void block_statement();
    void if_statement();
    void print_statement();
    void while_statement();
    void expression(Expr& expr);
    void assign_expr(Expr& expr);
    void compound_assign_expr(Expr& expr);
    void binary_expr(Expr& expr);
    void call_expr(Expr& expr);
    void literal_expr(Expr& expr);
    void logical_expr(Expr& expr);
    void unary_expr(Expr& expr);
    void variable_expr(Expr& expr);
    void begin_scope();
    void end_scope();
    void new_variable(Token& name);
    Local& resolve_variable(Token& name);
    size_t new_function(ObjFunction func);
    Local resolve_function(Token& name);
    void mark_initialized();
    bool match(StmtType type);
    void advance();
    bool is_at_end();
    void emit_constant(Value value);
    uint8_t make_constant(Value value);
    void emit_byte(uint8_t byte);
    void emit_bytes(uint8_t byte_1, uint8_t byte_2);
    void emit_loop(int loop_start);
    int emit_jump(uint8_t instruction);
    void patch_jump(int offset);
    void emit_return();
    void push_state(std::vector<StmtPtr> stmts);
    void pop_state();
    std::vector<StmtPtr>& stmts();
    Chunk& chunk();
    std::vector<Local>& locals();
    void push_locals();
    void pop_locals();
    StmtPtr& current();
    StmtPtr& previous();
    int& next();
    void write();

    std::vector<ObjFunction> functions;
    int current_function;

    std::vector<CompilerState> state_stack;

    std::vector<Value> constants;
    std::unordered_map<Value, uint8_t, ValueHash> constant_intern;
    std::string strings;
    std::unordered_map<std::string, uint8_t> string_intern;

    std::vector<std::vector<Local>> locals_stack;
    int scope_depth;

    FFI ffi;
};
#endif
