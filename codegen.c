#include "9cc.h"

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) error("代入の左辺値が変数ではありません");
    printf("    mov rax, rbp\n");
    if (node->offset >= 0) {
        printf("    sub rax, %d\n", node->offset);
    } else {
        printf("    add rax, %d\n", -node->offset);
    }
    printf("    push rax\n");
}

void gen_func(Function *func) {
    char *func_name = calloc(func->name_len + 1, sizeof(char));
    memcpy(func_name, func->name, func->name_len);
    // fprintf(stderr, "gen func: %s\n", func_name);
    // fprintf(stderr, "locals: %d\n", func->locals->size);
    // fprintf(stderr, "params: %d\n", func->params->size);
    printf(".globl %s\n", func_name);
    printf("%s:\n", func_name);
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", func->locals->size * 8);
    for (int i = 0; i < func->node->stmts->size; i++) {
        gen(func->node->stmts->data[i]);
    }
}

void gen_func_call(Node *node) {
    char *func_name = calloc(node->name_len + 1, sizeof(char));
    memcpy(func_name, node->name, node->name_len);
    // fprintf(stderr, "gen call:%s\n", func_name);
    for (int i = 0; i < node->args->size; i++) {
        Node *arg = (Node *)node->args->data[i];
        gen(arg);
    }
    printf("    call %s\n", func_name);
    printf("    add rsp, %d\n", node->args->size * 8);
    printf("    push rax\n");
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
            if (node->els) {
                printf("    je .Lelse%d\n", control_idx);
                gen(node->then);
                printf("    jmp .Lend%d\n", control_idx);
                printf(".Lelse%d:", control_idx);
                gen(node->els);
            } else {
                printf("    je .Lend%d\n", control_idx);
                gen(node->then);
            }
            printf(".Lend%d:\n", control_idx);
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
        // TODO: 怪しいかも
        case ND_BLOCK:
            // fprintf(stderr, "reached block\n");
            for (int i = 0; i < node->stmts->size; i++) {
                gen((Node *)node->stmts->data[i]);
                printf("    pop rax\n");
            }
            return;
        case ND_CALL:
            gen_func_call(node);
            return;
        case ND_FUNC:
            fprintf(stderr, "reached func\n");
            exit(1);
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