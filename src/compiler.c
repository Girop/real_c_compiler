#include <stdio.h>
#include <string.h>
#include <string.h>
#include "utils.h"
#include "x86.h"
#include "bytecode.h"


static char const* read_file(const char* filename)
{
    FILE* file = fopen(filename, "r");
    assert(file);
    
    fseek(file, 0, SEEK_END);
    long file_sz = ftell(file);
    rewind(file);
    char* file_content = cc_malloc(file_sz + 1);
    file_content[file_sz] = '\0';
    fread(file_content, 1, file_sz, file);
    fclose(file);
    return file_content;
}

// SSA as simplification:
// I want to introduce a intermediate representation very early, since
// a) as a way to decrease the number of types of operations
// b) and I want to perform codegen optimizations anyway
// OR 
// I could also produce bytecode with minimal instruction (with type tags) set for a hypothetical stack machine,
// and then, convert it to the specific architecture.


// Some todos:
// - fix argument parsing
// - x86 asm generation
// - gradual frontend development
// - tests

// Few rules first:
//  - Don't be afraid to be over-explicit in the AST design

// Testing ideas:
// - UTs for AST seem plausible
// - UTs for general data structures too
// - Bytecode based tests for compilation part
// - Collection of small sample programs 
struct InputFlags
{
    bool show_tokens;  
    bool show_ast;
    bool print_asm;
    char const* filename;
};

struct InputFlags handle_arguments(int argc, char** argv)
{
    struct InputFlags flags = {0};
    assert(argc >= 2 && "Missing input file");
    flags.filename = argv[1];

    for (int arg_idx = 1; arg_idx < argc; ++arg_idx)
    {
        if (strcmp(argv[arg_idx], "-t") == 0)
        {
            flags.show_tokens = true;    
        } 
        else if (strcmp(argv[arg_idx], "-a") == 0)
        {
            flags.show_ast = true;
        } 
    }
    return flags;
}

int main(int argc, char* argv[])
{
    struct InputFlags options = handle_arguments(argc, argv);
    const char* file_contents = read_file(options.filename);
    struct FunctionAst* ast = produce_ast(file_contents);
    
    struct VirtualMachineCode tape = compile_to_vm(ast);
    print_tape(&tape);
    struct StringArray assembly = codegen(&tape);
    UNUSED(assembly);
}

