#include <stdarg.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

void* cc_malloc(size_t sz)
{
    void* alloc = calloc(1, sz); // Dealing only with 0s is easier and can be abused :)
    assert(alloc);
    return alloc;
}

struct StringArray 
{
    char** data;
    size_t size;
    size_t max_capacity;
};

struct StringArray new_string_array()
{
    return (struct StringArray) {
        .data = cc_malloc(8 * sizeof(char*)),
        .size = 0,
        .max_capacity = 8
    };
}

void add_string(struct StringArray* arr, char const* string)
{
    if (arr->size + 1 > arr->max_capacity) {
        arr->max_capacity *= 1.4; 
        arr->data = realloc(arr->data, arr->max_capacity * sizeof(char*));
        assert(arr->data);
    }
    arr->data[arr->size] = strdup(string);
    ++arr->size;
}

#define DEFINE_NEW_DYN_ARRAY(ARRAY_NAME, TYPE, NEW_FUNC_NAME, ADD_FUNC_NAME) \
struct ARRAY_NAME  \
{ \
    TYPE* data; \
    size_t size; \
    size_t max_capacity; \
}; \
struct ARRAY_NAME NEW_FUNC_NAME() \
{  \
    return (struct ARRAY_NAME) { \
        .data = cc_malloc(8 * sizeof(TYPE)), \
        .size = 0, \
        .max_capacity = 8 \
    }; \
} \
void ADD_FUNC_NAME(struct ARRAY_NAME* arr, TYPE const* value) \
{ \
    if (arr->size + 1 > arr->max_capacity) { \
        arr->max_capacity *= 1.4;  \
        TYPE* new_data = realloc(arr->data, arr->max_capacity * sizeof(TYPE)); \
        assert(new_data); \
        arr->data = new_data; \
    } \
    arr->data[arr->size] = *value; \
    ++arr->size; \
}

// Hashing function implemantions from: https://stackoverflow.com/questions/7666509/hash-function-for-string
uint32_t murmurOAAT32(const char* key)
{
  uint32_t h = 3323198485ul;
  for (;*key;++key) {
    h ^= *key;
    h *= 0x5bd1e995;
    h ^= h >> 15;
  }
  return h;
}

uint64_t murmurOAAT64(const char* key)
{
  uint64_t h = 525201411107845655ull;
  for (;*key;++key) {
    h ^= *key;
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return h;
}

struct HashmapElem
{
    const char* key;
    int32_t value;
};

struct HashMap
{
    struct HashmapElem* data;
    size_t capacity;
    size_t size;
};

void hashmap_init(struct HashMap* map)
{
    map->capacity = 32;
    map->size = 0;
    map->data = cc_malloc(32 * sizeof(struct HashmapElem));
}

static void reallocate_hashmap(struct HashMap* map, size_t new_capacity)
{
    struct HashmapElem* new_data = cc_malloc(new_capacity * sizeof(struct HashmapElem));
    for (size_t idx = 0; idx < map->capacity; ++idx)
    {
        struct HashmapElem* item = &map->data[idx];
        if (item->key == NULL) continue;
        size_t new_index = murmurOAAT32(item->key) % new_capacity;
        while (new_data[new_index].key != NULL) new_index = (new_index + 1) % new_capacity;
        new_data[new_index] = *item;
    }
    free(map->data);
    map->data = new_data;
    map->capacity = new_capacity;
}

void hashmap_insert(struct HashMap* map, const char* key, int32_t value) 
{
    float load_factor = (float) map->size / map->capacity;
    if (load_factor > 0.7)
    {
        reallocate_hashmap(map, map->size * 1.4);
    }
    size_t index = murmurOAAT32(key) % map->capacity;
    while (map->data[index].key == NULL) index = (index + 1) % map->capacity;
    map->data[index] = (struct HashmapElem){ .key = strdup(key), .value = value};
}

// May return null
int32_t* hashmap_find(struct HashMap* map, const char* key)
{
    size_t index = murmurOAAT32(key) % map->capacity;
    while (map->data[index].key != NULL)
    {
        struct HashmapElem* elem = &map->data[index];
        if (strcmp(elem->key, key) == 0)
        {
            return &elem->value;
        }
        index = (index + 1) % map->capacity;
    }
    return NULL;
}

// void hashmap_erase(struct HashMap* map, const char* key);

__attribute__((format(printf, 1, 2)))
char* format(char const* format, ...)
{
    va_list args;
    va_start(args, format);
    size_t const size = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);

    assert(size > 1);
    char* message = cc_malloc(size);
    va_start(args, format);
    vsnprintf(message, size, format, args);
    va_end(args);
    return message;
}

enum TokenType {
    TOK_INVALID = 0,
    TOK_INT,
    TOK_INT_VALUE,
    TOK_RETURN,
    TOK_PLUS,
    TOK_LEFT_PAREN,
    TOK_RIGHT_PAREN,
    TOK_LEFT_BRACE,
    TOK_RIGHT_BRACE,
    TOK_EQ,
    TOK_SEMICOLON,
    TOK_NAME
};

struct Token {
    enum TokenType type;
    char* name;
    int value;
};

struct Token new_token(enum TokenType type)
{
    return (struct Token) {type, 0, 0};
}

const char* token_to_string(struct Token const* token)
{
    switch(token->type) 
    {
        case TOK_INVALID: return "INVALID TOKEN";
        case TOK_INT: return "int";
        case TOK_INT_VALUE: return format("%d", token->value);
        case TOK_RETURN: return "return";
        case TOK_PLUS: return "+";
        case TOK_LEFT_PAREN: return "(";
        case TOK_RIGHT_PAREN: return ")";
        case TOK_LEFT_BRACE: return "{";
        case TOK_RIGHT_BRACE: return "}";
        case TOK_EQ: return "=";
        case TOK_SEMICOLON: return ";";
        case TOK_NAME: return format("Name: %s", token->name);
    }
    return "Invalid token";
}


struct KeywordMapElem {
    char const* string;
    enum TokenType type;
};

struct KeywordMapElem keywords_or_builtin_types[] = {
    {"int", TOK_INT},
    {"return", TOK_RETURN},
};

struct Token lex_keyword(char const* input_stream, size_t* position)
{
    size_t start_pos = *position;
    while (isalnum(input_stream[*position])) ++(*position);
    size_t const map_elem_count = sizeof(keywords_or_builtin_types) / sizeof(struct KeywordMapElem);
    
    struct Token token = new_token(TOK_NAME);

    size_t keyword_size = *position - start_pos;
    --(*position);
    for (size_t index = 0; index < map_elem_count; ++index)
    {
        if (strncmp(keywords_or_builtin_types[index].string, &input_stream[start_pos], keyword_size) == 0)
        {
            token.type = keywords_or_builtin_types[index].type;
            break;
        }
    }

    if (token.type == TOK_NAME)
    {
        token.name = cc_malloc(keyword_size + 1);
        memcpy(token.name, &input_stream[start_pos], keyword_size);
    }

    return token;
}

struct Token* lex(char const* input_stream, size_t* token_count)
{
    // TODO: DYNARRAY
    struct Token* tokens = cc_malloc(100 * sizeof(struct Token));
    size_t positon = 0;
    size_t total_length = strlen(input_stream);
    assert(total_length > 0);

    while (positon < total_length)
    {
        if (isspace(input_stream[positon]))
        {
            ++positon;
            continue;
        }
    
        if (isalpha(input_stream[positon]))
        {
            tokens[*token_count] = lex_keyword(input_stream, &positon);
            goto NEW_TOK_END;
        }

        if (isdigit(input_stream[positon]))
        {
            size_t const start_positon = positon;
            while(isdigit(input_stream[positon++]));
            size_t const length = positon - start_positon + 1;
            char* tmp = cc_malloc(length + 1);
            memcpy(tmp, &input_stream[start_positon], length);
            struct Token* current_token = &tokens[*token_count];
            current_token->type = TOK_INT_VALUE;
            current_token->value = strtod(&input_stream[start_positon], &tmp);
            // TODO temp fix
            --positon;
            ++(*token_count);
            continue;
        }

        struct Token* current_token = &tokens[*token_count];
        switch (input_stream[positon])
        {
            case '(':
                current_token->type = TOK_LEFT_PAREN;
                goto NEW_TOK_END;
            case ')':
                current_token->type = TOK_RIGHT_PAREN;
                goto NEW_TOK_END;
            case '+':
                current_token->type = TOK_PLUS;
                goto NEW_TOK_END;
            case ';':
                current_token->type = TOK_SEMICOLON;
                goto NEW_TOK_END;
            case '=':
                current_token->type = TOK_EQ;
                goto NEW_TOK_END;
            case '{':
                current_token->type = TOK_LEFT_BRACE;
                goto NEW_TOK_END;
            case '}':
                current_token->type = TOK_RIGHT_BRACE;
                goto NEW_TOK_END;
            default:
                current_token->type = TOK_INVALID;
                goto NEW_TOK_END;
        }

NEW_TOK_END:
        ++positon;
        ++(*token_count);
    }

    assert(*token_count < 100);
    return tokens;
}


char const* read_file(const char* filename)
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


enum ValueType 
{
    TYPE_INT,
};

struct BinaryExpression;

struct ExpressionNode 
{
    enum AstExpressionType 
    {
        EXPR_VARIABLE,
        EXPR_CONSTANT,
        EXPR_BIN,
    } type;
    union {
        struct BinaryExpression* bin;
        struct Token* simple;
    } as;
};

struct BinaryExpression
{
    enum BinaryOp 
    {
        BIN_ADD
    } op;
    struct ExpressionNode* left; 
    struct ExpressionNode* right; 
};

struct ReturnNode 
{
    struct ExpressionNode* value;   
};

struct VariableAssignment 
{
    struct Token* name;
    struct ExpressionNode* value;
};

struct DefineVariable 
{
    struct Token* name;
    enum ValueType type;
    bool has_inital_value; 
    struct ExpressionNode* value;
};

struct StatementAst 
{
    enum StatementTag 
    {
        TAG_DEFINITION,
        TAG_ASSIGMENT,
        TAG_RETURN
    } tag;
    union {
        struct DefineVariable definition;
        struct VariableAssignment assignement;
        struct ReturnNode ret;
    } as;
};

struct FunctionAst {
    enum ValueType return_type;
    struct Token* name;    
    // Body
    struct StatementAst** statements;
    size_t statement_count;
};


struct Parser {
    size_t token_count;
    struct Token* tokens; 
    size_t current_position;
} parser;


struct Token* current_token()
{
    return &parser.tokens[parser.current_position];
}


void progress_tokens()
{
    ++parser.current_position;
    assert(parser.current_position <= parser.token_count);
}

struct Token* consume_token()
{
    struct Token* token = current_token();
    progress_tokens();
    return token;
}

void consume_expected(enum TokenType expected)
{
    struct Token* token = current_token();
    if(token->type != expected) 
    {
        printf(
            "Token: %s, expected type: %d, but got %d\n",
            token_to_string(token),
            expected, token->type);
        exit(1);
    }
    progress_tokens();
}

struct Token* get_expected(enum TokenType expected)
{
    consume_expected(expected);
    return &parser.tokens[parser.current_position - 1];
}

bool get_if_expected(enum TokenType expected, struct Token** res_token)
{
    if (current_token()->type != expected) return false;
    assert(res_token != NULL);
    *res_token = current_token();
    progress_tokens();
    return true;
}

bool consume_if_expected(enum TokenType expected)
{
    struct Token* token;
    return get_if_expected(expected, &token);
}

enum ValueType parse_type()
{
    consume_expected(TOK_INT);
    return TYPE_INT;
}


struct ExpressionNode* parse_simple_expression()
{
    struct ExpressionNode* expr = cc_malloc(sizeof(struct ExpressionNode));
    struct Token* matched;
    if (get_if_expected(TOK_NAME, &matched))
    {
        expr->type = EXPR_VARIABLE;
    } 
    else if (get_if_expected(TOK_INT_VALUE, &matched))
    {
        expr->type = EXPR_CONSTANT;
    }
    assert(matched != NULL);
    expr->as.simple = matched;
    return expr;
}

struct ExpressionNode* parse_binary_expression()
{
    struct ExpressionNode* simple_expression = parse_simple_expression();
    if (consume_if_expected(TOK_PLUS))  // Binary operators: +
    {
        struct BinaryExpression* binary_expr = cc_malloc(sizeof(struct BinaryExpression));
        binary_expr->op = BIN_ADD;
        binary_expr->left = simple_expression;
        binary_expr->right = parse_binary_expression();

        struct ExpressionNode* binar_but_expression = cc_malloc(sizeof(struct ExpressionNode));
        binar_but_expression->type = EXPR_BIN;
        binar_but_expression->as.bin = binary_expr;
        return binar_but_expression;
    }
    return simple_expression;
}

struct ExpressionNode* parse_expression()
{
    struct ExpressionNode* expr = parse_binary_expression(); // Highest level of expressions in grammar
    return expr;
}

struct StatementAst* parse_statement()
{
    //  For now a statement is either:
    //  a) variable declaration (begins with type)
    //  b) value assignement (begins with name)
    //  c) return value (begins with return)
    struct StatementAst* statement = cc_malloc(sizeof(struct StatementAst));
    struct Token* matched = NULL;
    if (get_if_expected(TOK_INT, &matched)) 
    {
        statement->tag = TAG_DEFINITION;

        struct Token* name = get_expected(TOK_NAME);
        statement->as.definition.name = name;
        statement->as.definition.type = TYPE_INT;
        if (consume_if_expected(TOK_EQ))
        {
            statement->as.definition.has_inital_value = true;
            statement->as.definition.value = parse_expression();
        } 
        else 
        {
            statement->as.definition.has_inital_value = false;
        }
    } 
    else if (get_if_expected(TOK_NAME, &matched)) 
    {
        consume_expected(TOK_EQ);
        statement->tag = TAG_ASSIGMENT; 
        statement->as.assignement.name = matched;
        statement->as.assignement.value = parse_expression();
    }
    else if (get_if_expected(TOK_RETURN, &matched)) 
    {
        statement->tag = TAG_RETURN;
        statement->as.ret.value = parse_expression();
    }
    assert(matched != NULL);
    consume_expected(TOK_SEMICOLON);
    return statement;
}

struct FunctionAst* parse_function()
{
    struct FunctionAst* function = cc_malloc(sizeof(struct FunctionAst));
    function->return_type = parse_type();
    function->name = get_expected(TOK_NAME);
    consume_expected(TOK_LEFT_PAREN);
    consume_expected(TOK_RIGHT_PAREN);
    consume_expected(TOK_LEFT_BRACE);
    function->statements = cc_malloc(100 * sizeof(struct StatementAst));
    while (!consume_if_expected(TOK_RIGHT_BRACE) || parser.current_position < parser.token_count)
    {
        function->statements[function->statement_count] = parse_statement();
        ++function->statement_count;
        assert(function->statement_count < 100);
    }
    return function;
}


struct FunctionAst* parse(struct Token* tokens, size_t token_count)
{
    parser.current_position = 0;
    parser.tokens = tokens;
    parser.token_count = token_count;
    return parse_function();
}

void print_depth_indicators(size_t depth)
{
    for (size_t depth_idx = 0; depth_idx < depth; ++depth_idx)
    {
        printf("-");
    }
    printf(" ");
}


void print_ast_expression(struct ExpressionNode const* expr, size_t depth);


char const* OPERATOR_REPR[] = {
    [BIN_ADD] = "+"
};

void print_binary_expr(struct BinaryExpression* binary_expr, size_t depth)
{
    printf("Binary expression, operator: %s\n", OPERATOR_REPR[binary_expr->op]);

    print_depth_indicators(depth + 1);
    printf("Left:\n");
    print_depth_indicators(depth + 2);
    print_ast_expression(binary_expr->left, depth + 1);

    print_depth_indicators(depth + 1);
    printf("Right:\n");
    print_depth_indicators(depth + 2);
    print_ast_expression(binary_expr->right, depth + 1);
}

void print_ast_expression(struct ExpressionNode const* expr, size_t depth)
{
    switch (expr->type) 
    {
        case EXPR_VARIABLE:
            printf("Variable: %s\n", expr->as.simple->name);
            break;
        case EXPR_CONSTANT:
            printf("Constant: %d\n", expr->as.simple->value);
            break;
        case EXPR_BIN:
            print_binary_expr(expr->as.bin, depth);
            break;
    }
}


void print_variable_definition(struct DefineVariable const* definition, size_t depth)
{
    printf("New variable: %s, type: %d\n", definition->name->name, definition->type);
    if (definition->has_inital_value)
    {
        print_depth_indicators(depth + 1);
        printf("With inital value: ");
        print_ast_expression(definition->value, depth);
    }
}


void print_variable_assignment(struct VariableAssignment const* assignement, size_t depth)
{
    printf("Assigning to variable: %s\n", assignement->name->name);
    if (assignement->value->type)
    {
        print_depth_indicators(depth + 1);
        print_ast_expression(assignement->value, depth + 1);
    }
}

void print_variable_return(struct ReturnNode const* ret, size_t depth)
{
    printf("Return\n");
    print_depth_indicators(depth + 1);
    print_ast_expression(ret->value, depth + 1);
}

void print_function_ast(struct FunctionAst const* ast)
{
    size_t depth = 0;
    printf("Function %s:\n", ast->name->name);
    ++depth;
    for (size_t idx = 0; idx < ast->statement_count; ++idx)
    {
        print_depth_indicators(depth);

        struct StatementAst* stmt = ast->statements[idx];
        switch (stmt->tag)
        {
            case TAG_DEFINITION:
                print_variable_definition(&stmt->as.definition, depth);
                break;
            case TAG_ASSIGMENT:
                print_variable_assignment(&stmt->as.assignement, depth);
                break;
            case TAG_RETURN:
                print_variable_return(&stmt->as.ret, depth);
                break;
        }
    }
}

void print_ast(struct FunctionAst const* ast)
{
    printf("\nPrinting debug AST representation\n\n");
    print_function_ast(ast);
}

void list_tokens(struct Token const* tokens, size_t token_count)
{
    for (size_t idx = 0; idx < token_count; ++idx) 
    {
        printf("Token: %s\n", token_to_string(&tokens[idx]));
    }
}


#define TABLE_SIZE 1024

struct VariableInTable 
{
    bool is_set;
    char const* name;
    enum ValueType type;
    size_t stack_offset;
};

struct SymbolTable 
{
    struct VariableInTable variables[TABLE_SIZE];
} table = {0};


size_t get_type_size(enum ValueType type)
{
    assert(type == TYPE_INT);
    return sizeof(int);
}

size_t calculate_stack_offset(size_t new_symbol_pos)
{
    size_t offset = 0;
    for (size_t idx = 0; idx < new_symbol_pos; ++idx)
    {
        offset += get_type_size(table.variables[idx].type);
    }
    return offset;
}


void add_variable_to_table(char const* name, enum ValueType type)
{
    assert(type == TYPE_INT);
    for (size_t idx = 0; idx < TABLE_SIZE; ++idx)
    {
        struct VariableInTable* var = &table.variables[idx];
        if (!var->is_set)
        {
            var->is_set = true;
            var->name = strdup(name);
            var->type = type;
            var->stack_offset = calculate_stack_offset(idx) + get_type_size(var->type);
            break;
        }
        else if (strcmp(var->name, name) == 0) 
        {
            printf("Symbol %s redefintion\n", name);
            exit(1);
        }
    }
}

struct VariableInTable* find_symbol(char const* name)
{
    struct VariableInTable* symbol = NULL;
    for (size_t idx = 0; idx < TABLE_SIZE; ++idx)
    {
        if (strcmp(table.variables[idx].name, name) == 0)
        {
            symbol = &table.variables[idx];
            break;
        }
    }
    assert(symbol != NULL);
    return symbol;
}

char const* variable_location(char const* name)
{
    struct VariableInTable* symbol = find_symbol(name);
    return format("dword [rbp - %d]", (int)symbol->stack_offset);
}

void read_variable_into_eax(struct StringArray* arr, char const* variable_name)
{
    char const* location = variable_location(variable_name);
    add_string(arr, format("mov eax, %s", location));
}

void on_expression(struct StringArray* arr, struct ExpressionNode* expre);


char const* binary_op_instructions[] = {
    [BIN_ADD] = "add",
};

void on_binary_expression(struct StringArray* arr, struct BinaryExpression* expr)
{
    char const* op = binary_op_instructions[expr->op];
    on_expression(arr, expr->right);
    add_string(arr, "mov rdi, rax");
    on_expression(arr, expr->left);
    add_string(arr, format("%s rax, rdi", op));
}

// put result in eax
void on_expression(struct StringArray* arr, struct ExpressionNode* expre)
{
    switch(expre->type)
    {
        case EXPR_CONSTANT: 
        {
            struct Token* token = expre->as.simple;
            add_string(arr, format("mov eax, %d", token->value));
            break;
        }
        case EXPR_VARIABLE: 
        {
            struct Token* token = expre->as.simple;
            add_string(arr, format("mov eax, %s", variable_location(token->name)));
            break;
        }
        case EXPR_BIN: 
        {
            on_binary_expression(arr, expre->as.bin);
            break;
        }
    }
}


void on_definition(struct StringArray* arr, struct DefineVariable* definition)
{
    char const* new_var_name = definition->name->name;
    add_variable_to_table(new_var_name, definition->type);
    if (definition->has_inital_value) 
    {
        on_expression(arr, definition->value);
        add_string(arr, format("mov %s, eax", variable_location(new_var_name)));
    }
}

void on_assignment(struct StringArray* arr, struct VariableAssignment* asgn)
{
    on_expression(arr, asgn->value);
    char const* name = asgn->name->name;
    add_string(arr, format("mov %s, eax", variable_location(name)));
}

void on_return(struct StringArray* arr, struct ReturnNode* ret)
{
    on_expression(arr, ret->value);
    add_string(arr, "pop rbp");
    add_string(arr, "ret");
}

void codegen_function(struct StringArray* arr, struct FunctionAst* ast)
{
    // Ignoring the return value for now, assume all the world is built upon ints
    // Function name:
    char* res = format("%s:", ast->name->name);
    add_string(arr, res); 

    add_string(arr, "push rbp");
    add_string(arr, "mov rbp, rsp");
    
    for (size_t idx = 0; idx < ast->statement_count; ++idx)
    {
        struct StatementAst* stmt = ast->statements[idx];
        switch(ast->statements[idx]->tag)
        {
            case TAG_ASSIGMENT:
                on_assignment(arr, &stmt->as.assignement);
                break;
            case TAG_DEFINITION:
                on_definition(arr, &stmt->as.definition);
                break;
            case TAG_RETURN:
                on_return(arr, &stmt->as.ret);
                break;
        }
    }
    // Functions must contain explicit return
    // add_string(arr, "pop rbp");
    // add_string(arr, "ret");
}



struct StringArray codegen(struct FunctionAst* ast)
{
    struct StringArray arr = new_string_array();
    add_string(&arr, "section .text");
    add_string(&arr, "global main");

    codegen_function(&arr, ast);
    add_string(&arr, "\nsection .note.GNU-stack noalloc noexec nowrite progbits"); // security note
    return arr;
}

void show_assembly(struct StringArray const* assembly)
{
    for (size_t idx = 0; idx < assembly->size; ++idx)
    {
        printf("%s\n", assembly->data[idx]);
    }
}


// SSA as simplification:
// I want to introduce a intermediate representation very early, since
// a) as a way to decrease the number of types of operations
// b) and I want to perform codegen optimizations anyway
// OR 
// I could also produce bytecode with minimal instruction (with type tags) set for a hypothetical stack machine,
// and then, convert it to the specific architecture.


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

char const* op_to_string(enum BytecodeOp op)
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

struct Local 
{
    const char* name;
    size_t depth;
};

union Bytecode
{
    enum BytecodeOp op;
    int value; 
};

DEFINE_NEW_DYN_ARRAY(Tape, union Bytecode, new_tape, add_to_tape)

struct VirtualMachineCode
{
    const char* symbol;
    struct Tape tape; 
    size_t stack_top;
    int32_t current_offset;
    // TODO Scoping rules... + split it up into actuall offstet map and a variable table with variable information
    struct HashMap stack_offsets;
};


void push_ins(struct VirtualMachineCode* vm, enum BytecodeOp op)
{
    union Bytecode code;
    code.op = op;
    add_to_tape(&vm->tape, &code);
}

void push_constant(struct VirtualMachineCode* vm, int value)
{
    union Bytecode code;
    code.value = value;
    add_to_tape(&vm->tape, &code);
}

void compile_expression(struct VirtualMachineCode* vm, struct ExpressionNode const* expr);

void compile_binary_expression(struct VirtualMachineCode* vm, struct BinaryExpression const* expr)
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

void compile_expression(struct VirtualMachineCode* vm, struct ExpressionNode const* expr)
{
    switch (expr->type) 
    {
        case EXPR_CONSTANT:
            push_ins(vm, PUSH);
            push_constant(vm, expr->as.simple->value);
            break;
        case EXPR_VARIABLE:
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

void compile_assignment(struct VirtualMachineCode* vm, struct VariableAssignment const* assign)
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

void compile_var_definition(struct VirtualMachineCode* vm, struct DefineVariable const* def)
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

void compile_return(struct VirtualMachineCode* vm, struct ReturnNode const* ret)
{
    compile_expression(vm, ret->value);
    push_ins(vm, RET);
}

struct VirtualMachineCode compile_to_vm(struct FunctionAst const* ast)
{
    struct VirtualMachineCode vm;
    vm.symbol = ast->name->name;
    vm.current_offset = 0;
    hashmap_init(&vm.stack_offsets);

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

bool is_op_double_width(enum BytecodeOp op)
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


// Few rules first:
//  - I actually finish it this time, hence I must go fast and dirty
//  - Everything in the same file, because I always put too much effort into the whole 
//  architecture 'design' while not having too many features
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

    size_t token_count = 0;
    struct Token* tokens = lex(file_contents, &token_count);
    assert(token_count != 0 && tokens != NULL);

    if (options.show_tokens) 
    {
        list_tokens(tokens, token_count);
    }
    
    struct FunctionAst* ast = parse(tokens, token_count);
    print_ast(ast);

    struct VirtualMachineCode vm = compile_to_vm(ast);
    print_tape(&vm);
    return 0;
    // struct StringArray arr = codegen(ast);
    // show_assembly(&arr);
    if (options.print_asm) 
    {
        print_ast(ast);
    }
    struct StringArray arr = codegen(ast);
    show_assembly(&arr);
    return 0;
}

