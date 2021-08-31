#include "9cc.h"

Function *cur_func;

bool is_array(Node *node) {
    for (int i = 0; i < cur_func->locals->size; i++) {
        LVar *var = cur_func->locals->data[i];
        if (var->len == node->name_len && !memcmp(node->name, var->name, var->len)) return var->ty->ty == TP_ARRAY;
    }
    for (int i = 0; i < cur_func->params->size; i++) {
        LVar *var = cur_func->params->data[i];
        if (var->len == node->name_len && !memcmp(node->name, var->name, var->len)) return var->ty->ty == TP_ARRAY;
    }
    return false;
    // error("値がみつかりませんでした%s\n", node->name);
}

void gen_lval_deref(Node *node, bool is_root) {
    if (node->kind == ND_DEREF) {
        gen_lval_deref(node->lhs, false);
        if (!is_root) {  // 値として入っているアドレスを吐かせる
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
        }
    } else {
        if (node->ty->ty != TP_PTR) error("ポインタではありません\n");
        if (node->kind != ND_LVAR) {
            gen(node);
            return;
        }
        // 値として入っているアドレスを吐かせる
        printf("    mov rax, rbp\n");
        if (node->offset >= 0)
            printf("    sub rax, %d\n", node->offset);
        else
            printf("    add rax, %d\n", -node->offset);
        if (is_array(node)) {
            printf("    push rax\n");
            return;
        }
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
    }
}

void gen_global_def(LVar *global) {
    char *name = calloc(global->len + 1, sizeof(char));
    memcpy(name, global->name, global->len);
    printf("    .comm	%s, %d\n", name, calc_size(global->ty->ty));
}

void gen_global(Node *node) {
    char *name = calloc(node->name_len + 1, sizeof(char));
    memcpy(name, node->name, node->name_len);
    printf("    lea rax, %s[rip]\n", name);
    printf("    push rax\n");
}

void gen_lval(Node *node) {
    if (node->kind == ND_DEREF) {
        gen_lval_deref(node, true);
        return;
    }
    if (node->kind != ND_LVAR) error("代入の左辺値が変数ではありません");
    if (node->is_global) {
        gen_global(node);
        return;
    }
    printf("    mov rax, rbp\n");
    if (node->offset >= 0) {
        printf("    sub rax, %d\n", node->offset);
    } else {
        printf("    add rax, %d\n", -node->offset);
    }
    printf("    push rax\n");
}

void gen_func(Function *func) {
    cur_func = func;
    char *func_name = calloc(func->name_len + 1, sizeof(char));
    memcpy(func_name, func->name, func->name_len);
    printf(".globl %s\n", func_name);
    printf("%s:\n", func_name);
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", sum_offset(func->locals));
    for (int i = 0; i < func->node->stmts->size; i++) {
        gen(func->node->stmts->data[i]);
    }
    // return節でrbpがもとに戻るのでここでは書かなくていい
}

void gen_func_call(Node *node) {
    char *func_name = calloc(node->name_len + 1, sizeof(char));
    memcpy(func_name, node->name, node->name_len);
    for (int i = 0; i < node->args->size; i++) {
        Node *arg = (Node *)node->args->data[i];
        gen(arg);
    }
    printf("    call %s\n", func_name);
    printf("    add rsp, %d\n", node->args->size * 8);
    printf("    push rax\n");
}

void gen(Node *node) {
    int size = -1;
    int control_idx = -1;
    switch (node->kind) {
        case ND_NUM:
            printf("    push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            size = calc_size(node->ty->ty);
            if (is_array(node)) return;
            printf("    pop rax\n");
            if(size == 8) printf("    mov rax, [rax]\n");
            else if(size == 4) printf("    mov eax, [rax]\n");
            else if(size == 1) printf("    movzx eax, BYTE PTR [rax]\n");
            else {
                fprintf(stderr, "対応していないサイズ: %d\n", size);
                exit(1);
            }
            printf("    push rax\n");
            return;
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            size = calc_size(node->lhs->ty->ptr_to->ty);
            printf("    pop rax\n");
            if(size == 8) printf("    mov rax, [rax]\n");
            else if(size == 4) printf("    mov eax, [rax]\n");
            else if(size == 1) printf("    movzx eax, BYTE PTR [rax]\n");
            else {
                fprintf(stderr, "対応していないサイズ: %d\n", size);
                exit(1);
            }
            printf("    push rax\n");
            return;
        case ND_ASSIGN:
            size = calc_size(node->lhs->ty->ty);
            gen_lval(node->lhs);
            gen(node->rhs);
            printf("    pop rdi\n");
            printf("    pop rax\n");
            if(size == 8) printf("    mov [rax], rdi\n");
            else if(size == 4) printf("    mov [rax], edi\n");
            else if(size == 1) printf("    mov [rax], dil\n");
            else {
                fprintf(stderr, "対応していないサイズ: %d\n", size);
                exit(1);
            }
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
            printf(".Lend%d:\n", control_idx);
            control_idx++;
            return;
        // TODO: 怪しいかも
        case ND_BLOCK:
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
            if (node->lhs->ty != NULL && node->lhs->ty->ty == TP_PTR) {
                printf("    imul rdi, %d\n", calc_size(node->lhs->ty->ptr_to->ty));
            }
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            if (node->lhs->ty != NULL && node->lhs->ty->ty == TP_PTR) {
                printf("    imul rdi, %d\n", calc_size(node->lhs->ty->ptr_to->ty));
            }
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