#include <stdio.h>
#include "9cc.h"

// FOR DEBUG
char *node_kind_to_char(NodeKind kind){
    if(kind == ND_ADD) return "+";
    if(kind == ND_SUB) return "-";
    if(kind == ND_MUL) return "*";
    if(kind == ND_DIV) return "/";
    if(kind == ND_EQ) return "==";
    if(kind == ND_NE) return "!=";
    if(kind == ND_LT) return "<";
    if(kind == ND_LE) return "<=";
}

void print_node(Node * node){
    if(node->kind == ND_NUM){
        printf("%d", node->val);
        return;
    }
    printf("(");
    print_node(node->lhs);
    printf("%s", node_kind_to_char(node->kind));
    print_node(node->rhs);
    printf(")");
}

int main(int argc, char** argv){
    if(argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    user_input = argv[1];
    token = tokenize();
    program();
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    // 変数26個分の領域を確保
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");
    for(int i = 0; code[i]; i++){
        gen(code[i]);
        printf("    pop rax\n");
    }
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}