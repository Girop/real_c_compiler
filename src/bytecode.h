#pragma once
#include <stdbool.h>
#include "utils.h"
#include "frontend.h"

enum BytecodeOp
{
    // Data movement
    PUSH, // Uses second operand as literal
    POP, // decrement stack
    LOAD, // Treat next value in ins tape as locaiton at push unerlying value on the stack
    STORE, // Target location as next instruction, value on the stack 
    // Data transformation
    // Consume first element and:
    NOT, // logical negation 
    // Consume top 2 stack elements and:
    ADD, // adds
    SUB, // subtracts
    MUL, // multiplies
    DIV, // divides 
    REM, // calculates modulo of first by second
    LSHIFT, // shifts first number left by second
    RSHIFT, // shifts first number right by second
        //  Not supported yet instructions
    // Flow
    // CMP, 
    // JMP,
    // JE,
    // JNE,
    // JG,
    // JL,
    CALL,
    RET
};

union Bytecode
{
    enum BytecodeOp op;
    int value; 
};

DEFINE_NEW_DYN_ARRAY(Tape, union Bytecode, new_tape, add_to_tape);

struct VirtualMachineCode
{
    const char* symbol;
    struct Tape tape; 
    size_t current_stack_offset;
    int32_t current_offset;
    // TODO Scoping rules... + split it up into actuall offstet map and a variable table with variable information
    struct HashMap stack_offsets;
};

struct VirtualMachineCode compile_to_vm(struct FunctionAst const* ast);
void print_tape(struct VirtualMachineCode const* vm);
