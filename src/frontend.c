#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "frontend.h"
#include "utils.h"

static char const* token_to_string(struct Token const* token)
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


static struct Token new_token(enum TokenType type)
{
    return (struct Token) {type, 0, 0};
}

struct KeywordMapElem 
{
    char const* string;
    enum TokenType type;
};

static struct KeywordMapElem keywords_or_builtin_types[] = {
    {"int", TOK_INT},
    {"return", TOK_RETURN},
};

static struct Token lex_keyword(char const* input_stream, size_t* position)
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

static struct Token* lex(char const* input_stream, size_t* token_count)
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

struct Parser 
{
    size_t token_count;
    struct Token* tokens; 
    size_t current_position;
} parser;


static struct Token* current_token()
{
    return &parser.tokens[parser.current_position];
}


static void progress_tokens()
{
    ++parser.current_position;
    assert(parser.current_position <= parser.token_count);
}

static struct Token* consume_token()
{
    struct Token* token = current_token();
    progress_tokens();
    return token;
}

static void consume_expected(enum TokenType expected)
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

static struct Token* get_expected(enum TokenType expected)
{
    consume_expected(expected);
    return &parser.tokens[parser.current_position - 1];
}

static bool get_if_expected(enum TokenType expected, struct Token** res_token)
{
    if (current_token()->type != expected) return false;
    assert(res_token != NULL);
    *res_token = consume_token();
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

static struct FunctionAst* parse_function()
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


static struct FunctionAst* parse(struct Token* tokens, size_t token_count)
{
    parser.current_position = 0;
    parser.tokens = tokens;
    parser.token_count = token_count;
    return parse_function();
}


struct FunctionAst* produce_ast(char const* text)
{
    assert(text != NULL);
    size_t token_count = 0;
    struct Token* tokens = lex(text, &token_count);
    assert(token_count != 0);
    // TODO List tokens


    struct FunctionAst* ast = parse(tokens, token_count);
    // print_ast(ast);
    return ast;
}

static void print_depth_indicators(size_t depth)
{
    for (size_t depth_idx = 0; depth_idx < depth; ++depth_idx)
    {
        printf("-");
    }
    printf(" ");
}


static void print_ast_expression(struct ExpressionNode const* expr, size_t depth);


static char const* OPERATOR_REPR[] = {
    [BIN_ADD] = "+"
};

static void print_binary_expr(struct BinaryExpression* binary_expr, size_t depth)
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

static void print_ast_expression(struct ExpressionNode const* expr, size_t depth)
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


static void print_variable_definition(struct DefineVariable const* definition, size_t depth)
{
    printf("New variable: %s, type: %d\n", definition->name->name, definition->type);
    if (definition->has_inital_value)
    {
        print_depth_indicators(depth + 1);
        printf("With inital value: ");
        print_ast_expression(definition->value, depth);
    }
}


static void print_variable_assignment(struct VariableAssignment const* assignement, size_t depth)
{
    printf("Assigning to variable: %s\n", assignement->name->name);
    if (assignement->value->type)
    {
        print_depth_indicators(depth + 1);
        print_ast_expression(assignement->value, depth + 1);
    }
}

static void print_variable_return(struct ReturnNode const* ret, size_t depth)
{
    printf("Return\n");
    print_depth_indicators(depth + 1);
    print_ast_expression(ret->value, depth + 1);
}

static void print_function_ast(struct FunctionAst const* ast)
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

