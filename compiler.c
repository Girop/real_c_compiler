#include <assert.h>
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

struct StringArray {
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

void add_element(struct StringArray* arr, char const* string)
{
    if (arr->size + 1 > arr->max_capacity) {
        char** new_data = realloc(arr->data, arr->max_capacity * 1.4);
        assert(new_data);
        arr->data = new_data;    
    }
    arr->data[arr->size] = strdup(string);
    ++arr->size;
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
        case TOK_INT_VALUE:;
            char* buffer = cc_malloc(100);
            sprintf(buffer, "%d", token->value);
            return buffer;
        case TOK_RETURN: return "return";
        case TOK_PLUS: return "+";
        case TOK_LEFT_PAREN: return "(";
        case TOK_RIGHT_PAREN: return ")";
        case TOK_LEFT_BRACE: return "{";
        case TOK_RIGHT_BRACE: return "}";
        case TOK_EQ: return "=";
        case TOK_SEMICOLON: return ";";
        case TOK_NAME:;
            size_t const len1 = strlen(token->name);
            size_t const len2 = strlen("Variable: ");
            char* name = cc_malloc(len1 + len2 + 1);
            char* result = strcpy(name, "Variable: ");
            return strcat(result, token->name);
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

enum StatementTag 
{
    TAG_DEFINITION,
    TAG_ASSIGMENT,
};

struct ExpressionNode 
{
    // Much work       
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
    bool has_inital_value; 
    struct ExpressionNode* value;
};

struct StatementAst 
{
    
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
    struct FunctionAst* result_ast;
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

struct StatementAst* parse_statement()
{
       
}

void parse_function()
{
    struct Token* token = current_token();
    struct FunctionAst* function = parser.result_ast;
    function->return_type = parse_type();
    function->name = get_expected(TOK_NAME);
    consume_expected(TOK_LEFT_PAREN);
    consume_expected(TOK_RIGHT_PAREN);
    consume_expected(TOK_LEFT_BRACE);
    
    while (!consume_if_expected(TOK_RIGHT_BRACE) || parser.current_position < parser.token_count)
    {
        function->statements[function->statement_count] = parse_statement();
        ++function->statement_count;
    }
}


struct FunctionAst* parse(struct Token* tokens, size_t token_count)
{
    parser.current_position = 0;
    parser.tokens = tokens;
    parser.token_count = token_count;
    parser.result_ast = cc_malloc(sizeof(struct FunctionAst));
    parse_function();
    return parser.result_ast;
}

void print_ast(struct FunctionAst* ast)
{
    printf("Parsing successful\n");
    for (size_t idx = 0; idx < ast->statement_count; ++idx)
    {
        // TODO
    }
}

// Few rules first:
//  - I actually finish it this time, hence I must go fast and dirty
//  - Everything in the same file, because I always put too much effort into the whole 
//  architecture 'design' while not having too many features
int main(int argc, char* argv[])
{
    assert(argc == 2);
    char const* filename = argv[1];
    const char* file_contents = read_file(filename);

    size_t token_count = 0;
    struct Token* tokens = lex(file_contents, &token_count);
    assert(token_count != 0 && tokens != NULL);
    

    for (size_t idx = 0; idx < token_count; ++idx) 
    {
        printf("Token: %s\n", token_to_string(&tokens[idx]));
    }
    struct FunctionAst* ast = parse(tokens, token_count);
    print_ast(ast);
    return 0;
}

