#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


void* cc_malloc(size_t sz)
{
    void* alloc = calloc(1, sz); // Dealing only with 0s is easier and can be abused :)
    assert(alloc);
    return alloc;
}


enum TokenType {
    TOK_INVALID = 0,
    TOK_INT,
    TOK_RETURN,
    TOK_PLUS,
    TOK_LEFT_PAREN,
    TOK_RIGHT_PAREN,
    TOK_LEFT_BRACE,
    TOK_RIGHT_BRACE,
    TOK_EQ,
    TOK_SEMICOLON,
    TOK_VARIABLE
};

struct Token {
    enum TokenType type;
    const char* name;
};

struct Token new_token(enum TokenType type)
{
    return (struct Token) {type, 0};
}


const char* tok_type_string(struct Token const* token)
{
    switch(token->type) 
    {
        case TOK_INVALID: return "Invalid token";
        case TOK_INT: return "int";
        case TOK_RETURN: return "return";
        case TOK_PLUS: return "+";
        case TOK_LEFT_PAREN: return "(";
        case TOK_RIGHT_PAREN: return ")";
        case TOK_LEFT_BRACE: return "{";
        case TOK_RIGHT_BRACE: return "}";
        case TOK_EQ: return "=";
        case TOK_SEMICOLON: return ";";
        case TOK_VARIABLE:;
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
    
    struct Token token = new_token(TOK_VARIABLE);

    size_t keyword_size = *position - start_pos;
    for (size_t index = 0; index < map_elem_count; ++index)
    {
        if (strncmp(keywords_or_builtin_types[index].string, &input_stream[start_pos], keyword_size) == 0)
        {
            token.type = keywords_or_builtin_types[index].type;
            break;
        }
    }
    return token;
}

struct Token* lex(char const* input_stream, size_t* token_count)
{
    // TODO: DYNARRAY
    struct Token* tokens = cc_malloc(100 * sizeof(struct Token));
    size_t positon = 0;
    token_count = 0;
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
        assert(*token_count < 100);
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

    size_t tok_index = 0;
    struct Token* current_token;
    do {
        current_token = &tokens[tok_index++];
        printf("Token: %s\n", tok_type_string(current_token));
    } while (tokens[token_count].type != TOK_INVALID);
    return 0;
}
