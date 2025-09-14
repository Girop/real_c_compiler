#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

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

struct Token 
{
    enum TokenType type;
    char* name;
    int value;
};

enum ValueType 
{
    TYPE_INT,
};

static inline size_t get_type_size(enum ValueType type)
{
    assert(type == TYPE_INT);
    return sizeof(int);
}

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

struct FunctionAst* produce_ast(char const* text);
void print_ast(struct FunctionAst const* ast);
void list_tokens(struct Token const* tokens, size_t token_count);
