#include "9cc.h"

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) error("代入の左辺値が変数ではありません");
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen_func(Function *func){
    // TODO: いい方法ないんか？
    fprintf(stderr, "gen func:");
    for(int i = 0; i < func->name_len; i++) fprintf(stderr, "%c", func->name[i]);
    fprintf(stderr, "\n");
    gen(func->node);
}

void gen(Node *node) {
    int control_idx = -1;
    switch (node->kind) {
        case ND_NUM:
            printf("    push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);
            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        case ND_IF:
            gen(node->cond);
            control_idx = n_controls++;
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            if(node->els){
                printf("    je .Lelse%d\n", control_idx);
                gen(node->then);
                printf("    jmp .Lend%d\n", control_idx);
                printf(".Lelse%d:", control_idx);
                gen(node->els);
            }else{
                printf("    je .Lend%d\n", control_idx);
                gen(node->then);
            }
            printf(".Lend%d:", control_idx);
            return;
        case ND_FOR:
            gen(node->init);
            control_idx = n_controls++;
            printf(".Lbegin%d:", control_idx);
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", control_idx);
            gen(node->body);
            gen(node->inc);
            printf("    jmp .Lbegin%d\n", control_idx);
            printf(".Lend%d:", control_idx);
            return;
        case ND_WHILE:
            control_idx = n_controls++;
            printf(".Lbegin%d:", control_idx);
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", control_idx);
            gen(node->body);
            printf("    jmp .Lbegin%d\n", control_idx);
            printf(".Lend%d:", control_idx);
            control_idx++;
            return;
        case ND_BLOCK:
            fprintf(stderr, "reached block\n");
            for(int i = 0; i < node->stmts->size; i++){
                gen((Node*)node->stmts->data[i]);
                printf("    pop rax\n");
            }
            return;
        case ND_CALL:
            fprintf(stderr, "reached call\n");
            return;
        case ND_FUNC:
            fprintf(stderr, "reached func\n");
            return;
    }
    gen(node->lhs);
    gen(node->rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    switch (node->kind) {
        case ND_EQ:
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_NE:
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LT:
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LE:
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
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
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
    }
    printf("    push rax\n");
}