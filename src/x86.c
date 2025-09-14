#include "x86.h"


void codegen_function(struct VirtualMachineCode const* tape)
{
    UNUSED(tape);
}

struct StringArray codegen(struct VirtualMachineCode const* tape)
{
    struct StringArray arr = new_string_array();
    add_string(&arr, "section .text");
    add_string(&arr, "global main");

    codegen_function(tape);
    char const* security_note =  "\nsection .note.GNU-stack noalloc noexec nowrite progbits";
    add_string(&arr, security_note);
    return arr;
}
