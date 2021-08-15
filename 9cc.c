#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

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
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val ){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Token *token;

bool consume(char op);
int expect_number();

Node *primary();
Node *mul();
Node *expr();

Node *expr(){
    Node *node = mul();
    while(true){
        if(consume('+')){
            node = new_node(ND_ADD, node, mul());
        }else if(consume('-')){
            node = new_node(ND_SUB, node, mul());
        }else{
            return node;
        }
    }
}

Node *mul(){
    Node *node = primary();
    while(true){
        if(consume('*')){
            node = new_node(ND_MUL, node, primary());
        }else if(consume('/')){
            node = new_node(ND_DIV, node, primary());
        }else{
            return node;
        }
    }
}

Node *primary(){
    if(consume('(')){
        Node *node = expr();
        consume(')');
        return node;
    }
    return new_node_num(expect_number());
}

void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char op){
    if(token->kind != TK_RESERVED || token->str[0] != op){
        return false;
    }
    token = token->next;
    return true;
}

void expect(char op){
    if(token->kind != TK_RESERVED || token->str[0] != op){
        error("'%c'ではありません", op);
    }
    token = token->next;
}

int expect_number(){
    if(token->kind != TK_NUM){
        error("数ではありません");
    }
    int ret = token->val;
    token = token->next;
    return ret;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p){
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while(*p){
        if(isspace(*p)){
            p++;
            continue;
        }

        if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'){
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("トークナイズ出来ません\n");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

// FOR DEBUG
char node_kind_to_char(NodeKind kind){
    if(kind == ND_ADD) return '+';
    if(kind == ND_SUB) return '-';
    if(kind == ND_MUL) return '*';
    if(kind == ND_DIV) return '/';
}

void print_node(Node * node){
    if(node->kind == ND_NUM){
        printf("%d", node->val);
        return;
    }
    printf("(");
    print_node(node->lhs);
    printf("%c", node_kind_to_char(node->kind));
    print_node(node->rhs);
    printf(")");
}

void gen(Node * node){
    if(node->kind == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    switch (node->kind){
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    
    }
    printf("    push rax\n");
}

int main(int argc, char** argv){
    if(argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    token = tokenize(argv[1]);
    Node *root = expr();
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    gen(root);
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}