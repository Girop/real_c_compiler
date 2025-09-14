#include <stdio.h>
#include "bytecode.h"


IMPLEMENT_NEW_DYN_ARRAY(Tape, union Bytecode, new_tape, add_to_tape);

static char const* op_to_string(enum BytecodeOp op)
{
    switch (op)
    {
        case PUSH: return "PUSH";
        case POP: return "POP";
        case LOAD: return "LOAD";
        case STORE: return "STORE";
        case NOT: return "NOT";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case REM: return "REM";
        case LSHIFT: return "LSHIFT";
        case RSHIFT: return "RSHIFT";
        case CALL: return "CALL";
        case RET: return "RET";
    }
    return "<UNDEFINED>";
}

static bool is_op_double_width(enum BytecodeOp op)
{
    return op == LOAD || op == STORE || op == PUSH;
}

void print_tape(struct VirtualMachineCode const* vm)
{
    printf("%s:\n", vm->symbol);
    for (size_t idx = 0; idx < vm->tape.size; ++idx)
    {
        union Bytecode byte = vm->tape.data[idx];
        printf("\t%s ", op_to_string(byte.op));
        if (is_op_double_width(byte.op))
        {
            printf("%d", vm->tape.data[++idx].value);
        }
        printf("\n");
    }
}

static void push_ins(struct VirtualMachineCode* vm, enum BytecodeOp op)
{
    union Bytecode code;
    code.op = op;
    add_to_tape(&vm->tape, &code);
}

static void push_constant(struct VirtualMachineCode* vm, int value)
{
    union Bytecode code;
    code.value = value;
    add_to_tape(&vm->tape, &code);
}

static void compile_expression(struct VirtualMachineCode* vm, struct ExpressionNode const* expr);

static void compile_binary_expression(struct VirtualMachineCode* vm, struct BinaryExpression const* expr)
{
    switch (expr->op)
    {
        case BIN_ADD:
            compile_expression(vm, expr->left);
            compile_expression(vm, expr->right);
            push_ins(vm, ADD);
            break;
    }
}

static void compile_expression(struct VirtualMachineCode* vm, struct ExpressionNode const* expr)
{
    switch (expr->type) 
    {
        case EXPR_CONSTANT:
            push_ins(vm, PUSH);
            push_constant(vm, expr->as.simple->value);
            break;
        case EXPR_VARIABLE:;
            int32_t* var = hashmap_find(&vm->stack_offsets, expr->as.simple->name);
            if (var == NULL)
            {
                printf("Usage of undefined variable: %s\n", expr->as.simple->name);
                exit(1);
            }
            push_ins(vm, LOAD);
            push_constant(vm, *var);
            break;
        case EXPR_BIN:
            compile_binary_expression(vm, expr->as.bin);
            break;
    }
}

static void compile_assignment(struct VirtualMachineCode* vm, struct VariableAssignment const* assign)
{
    int32_t* var = hashmap_find(&vm->stack_offsets, assign->name->name);
    if (var == NULL)
    {
        printf("Usage of undefined variable: %s\n", assign->name->name);
        exit(1);
    }
    compile_expression(vm, assign->value);
    push_ins(vm, STORE);
    push_constant(vm, *var);
}

static void compile_var_definition(struct VirtualMachineCode* vm, struct DefineVariable const* def)
{
    int32_t* elem = hashmap_find(&vm->stack_offsets, def->name->name);
    if (elem != NULL)
    {
        printf("Variable shadowing is not supported yet\n");
        exit(1);
    }

    size_t var_offset = vm->current_offset;
    hashmap_insert(&vm->stack_offsets, def->name->name, vm->current_offset);
    vm->current_offset += get_type_size(def->type);
    if (def->has_inital_value)
    {
        compile_expression(vm, def->value);
        push_ins(vm, STORE);
        push_constant(vm, var_offset);
    }
}

static void compile_return(struct VirtualMachineCode* vm, struct ReturnNode const* ret)
{
    compile_expression(vm, ret->value);
    push_ins(vm, RET);
}

struct VirtualMachineCode compile_to_vm(struct FunctionAst const* ast)
{
    struct VirtualMachineCode vm = {
        .symbol = ast->name->name,
        .tape = new_tape(),
        .current_offset = 0,
        .stack_offsets = new_hashmap(),
        .current_stack_offset = 0
    };
    assert(ast->return_type == TYPE_INT && "Supported only INTs");
    
    for (size_t stmt_idx = 0; stmt_idx < ast->statement_count; ++stmt_idx)
    {
        struct StatementAst const* stmt = ast->statements[stmt_idx];
        switch(stmt->tag)
        {
            case TAG_DEFINITION:
                compile_var_definition(&vm, &stmt->as.definition);
                break;
            case TAG_ASSIGMENT:
                compile_assignment(&vm, &stmt->as.assignement);
                break;
            case TAG_RETURN:
                compile_return(&vm, &stmt->as.ret);
                break;
        }
    }
    return vm;
}
