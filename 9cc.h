#ifndef NINECC_H
#define NINECC_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_FOR,
    TK_WHILE,
    TK_INT,
    TK_SIZEOF,
} TokenKind;

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_ADDR,
    ND_DEREF,
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_ASSIGN,
    ND_LVAR,
    ND_RETURN,
    ND_IF,
    ND_FOR,
    ND_WHILE,
    ND_BLOCK,
    ND_CALL,  // Function call
    ND_FUNC,  // Function definition
} NodeKind;

typedef enum {
    TP_INT,
    TP_PTR,
    TP_ARRAY,
} TypeKind;

typedef struct Token Token;
typedef struct Node Node;
typedef struct LVar LVar;
typedef struct Program Program;
typedef struct Function Function;
typedef struct Type Type;

struct Type {
    TypeKind ty;
    Type *ptr_to;
    size_t array_size;
};

struct Program {
    Vector *funcs;
};

struct Function {
    Vector *locals;
    char *name;
    int name_len;
    Node *node;
    Vector *params;
};

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset;  // 左辺値のときのみ使う
    Type *ty;

    // func name
    char *name;
    int name_len;

    // "if" ( cond ) then "else" els
    // "for" ( init; cond; inc ) body
    // "while" ( cond ) body
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;
    Node *body;

    // func call (vector of Node* (should be LVar or Num))
    Vector *args;
    // statements (vector of Node*)
    Vector *stmts;
};

struct LVar {
    char *name;
    int len;
    int offset;
    Type *ty;  // ここにも持つべきなのかなあ。。
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

Token *tokenize();

// parser
void *program();
Function *function();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unray();
Node *primary();

void gen(Node *node);
void gen_func(Function *func);

int calc_size(TypeKind ty);
Type *create_type(TypeKind kind, Type *ptr_to);

int sum_offset(Vector *locals);

Program *prog;
Token *token;
char *user_input;
int n_controls;  // 制御構文の番号

// parse中の関数に関するもの
Vector *params;
Vector *locals;

#endif