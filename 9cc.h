#ifndef NINECC_H
#define NINECC_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;

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
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

Token *tokenize();

bool consume(char *op);
int expect_number();

Node *primary();
Node *unray();
Node *mul();
Node *expr();
Node *equality();
Node *relational();
Node *add();

void gen(Node * node);

Token *token;
char *user_input;

#endif