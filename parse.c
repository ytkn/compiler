#include "9cc.h"
#include "vector.h"

char *token_kind_to_str(TokenKind kind) {
    if (kind == TK_INT) return "int";
    if (kind == TK_IDENT) return "識別子";
    return "please update token_kind_to_str";
}

void parse_error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    error_at(token->str, fmt, ap);
}

bool at_eof() {
    return token->kind == TK_EOF;
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_kind_of(TokenKind kind) {
    if (token->kind != kind) return NULL;
    Token *tok = token;
    token = token->next;
    return tok;
}

Token *consume_ident() {
    if (token->kind != TK_IDENT) return NULL;
    Token *tok = token;
    token = token->next;
    return tok;
}

void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

Token *expect_kind_of(TokenKind kind) {
    if (token->kind != kind)
        error_at(token->str, "%sではありません\n", token_kind_to_str(kind));
    Token *tok = token;
    token = token->next;
    return tok;
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    }
    int ret = token->val;
    token = token->next;
    return ret;
}

int calc_offset(Type *ty) {
    if (ty->ty == TP_ARRAY) return ty->array_size * calc_size(ty->ptr_to->ty);
    return 8;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    node->ty = create_type(TP_INT, NULL);
    return node;
}

Node *new_node_func_def() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC;
    node->stmts = create_vector();
    return node;
}

Node *new_node_func_call(char *name, int len) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CALL;
    node->name = name;
    node->name_len = len;
    node->args = create_vector();
    return node;
}

Node *new_node_lvar(LVar *lvar) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->name = lvar->name;
    node->name_len = lvar->len;
    node->ty = lvar->ty;
    node->offset = lvar->offset;
    return node;
}

Function *new_func(char *name, int len, Node *node) {
    Function *fn = calloc(1, sizeof(Function));
    fn->locals = create_vector();
    fn->params = create_vector();
    fn->name = name;
    fn->name_len = len;
    fn->node = node;
    return fn;
}

LVar *find_lvar(Token *tok) {
    for (int i = 0; i < locals->size; i++) {
        LVar *var = locals->data[i];
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) return var;
    }
    for (int i = 0; i < params->size; i++) {
        LVar *var = params->data[i];
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) return var;
    }
    return NULL;
}

LVar *create_lvar(char *name, int len, int offset, Type *ty) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->name = name;
    lvar->len = len;
    lvar->offset = offset;
    lvar->ty = ty;
    return lvar;
}

Type *create_type(TypeKind kind, Type *ptr_to) {
    Type *type = calloc(1, sizeof(Type));
    type->ty = kind;
    type->ptr_to = ptr_to;
    return type;
}

void *program() {
    prog = calloc(1, sizeof(Program));
    prog->funcs = create_vector();
    while (!at_eof()) {
        push_vector(prog->funcs, function());
    }
}

Function *function() {
    expect_kind_of(TK_INT);
    Token *tok = consume_ident();
    Node *node = new_node_func_def();
    Function *fn = new_func(tok->str, tok->len, node);
    locals = fn->locals;
    params = fn->params;
    expect("(");
    while (true) {
        if (consume_kind_of(TK_INT)) {
            Type *ty = create_type(TP_INT, NULL);
            while (consume("*")) {
                ty = create_type(TP_PTR, ty);
            }
            Token *param = expect_kind_of(TK_IDENT);
            LVar *lvar = create_lvar(param->str, param->len, 0, ty);
            push_vector(params, lvar);
            if (consume(",")) continue;
        }
        expect(")");
        break;
    }
    // offsetの調整
    for (int i = 0; i < params->size; i++) {
        LVar *var = params->data[i];
        var->offset = -(params->size + 1 - i) * 8;
    }
    expect("{");
    while (!consume("}")) {
        push_vector(node->stmts, stmt());
    }
    return fn;
}

Node *stmt() {
    if (consume_kind_of(TK_RETURN)) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
        return node;
    }

    if (consume_kind_of(TK_IF)) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume_kind_of(TK_ELSE)) {
            node->els = stmt();
        }
        return node;
    }

    if (consume_kind_of(TK_FOR)) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");
        node->init = expr();
        expect(";");
        node->cond = expr();
        expect(";");
        node->inc = expr();
        expect(")");
        node->body = stmt();
        return node;
    }

    if (consume_kind_of(TK_WHILE)) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = expr();
        expect(")");
        node->body = stmt();
        return node;
    }

    if (consume("{")) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->stmts = create_vector();
        while (true) {
            if (consume("}")) return node;
            push_vector(node->stmts, stmt());
        }
    }

    Node *node = expr();
    expect(";");
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality() {
    Node *node = relational();
    while (true) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = add();
    while (true) {
        if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else if (consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();
    while (true) {
        if (consume("+")) {
            if (node->kind == ND_LVAR && node->ty->ty == TP_ARRAY) {
                node->ty = create_type(TP_PTR, node->ty->ptr_to);
            }
            node = new_node(ND_ADD, node, mul());
            node->ty = node->lhs->ty;
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
            node->ty = node->lhs->ty;
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unray();
    while (true) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unray());
            node->ty = node->lhs->ty;
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unray());
            node->ty = node->lhs->ty;
        } else {
            return node;
        }
    }
}

Node *unray() {
    if (consume_kind_of(TK_SIZEOF)) {
        Node *arg = unray();
        if (arg->ty == NULL) parse_error("サイズが定義出来ません\n");
        return new_node_num(calc_size(arg->ty->ty));
    } else if (consume("-")) {
        Node *node = primary();
        node->val = -node->val;
        return node;
    } else if (consume("+")) {
        Node *node = primary();
        return node;
    } else if (consume("*")) {
        Node *val = unray();
        if (val->kind == ND_LVAR && val->ty->ty == TP_ARRAY) {  // いいのかなあ。
            val->ty = create_type(TP_PTR, val->ty->ptr_to);
        }
        Node *node = new_node(ND_DEREF, val, NULL);
        node->ty = node->lhs->ty;
        return node;
    } else if (consume("&")) {
        Node *node = new_node(ND_ADDR, unray(), NULL);
        node->ty = create_type(TP_PTR, NULL);
        return node;
    } else {
        Node *node = primary();
        if(consume("[")){ // ここでいいのかなあ
            if (node->kind == ND_LVAR && node->ty->ty == TP_ARRAY) {
                node->ty = create_type(TP_PTR, node->ty->ptr_to);
            }
            if(node->ty->ty != TP_PTR) parse_error("ポインタではありません\n");
            node = new_node(ND_ADD, node, add());
            node->ty = node->lhs->ty;
            node = new_node(ND_DEREF, node, NULL);
            node->ty = node->lhs->ty;
            expect("]");
        }
        return node;
    }
}

// なんか汚いよなあ。。
Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    if (consume_kind_of(TK_INT)) {
        Type *ty = create_type(TP_INT, NULL);
        while (consume("*")) {
            ty = create_type(TP_PTR, ty);
        }

        Token *tok = expect_kind_of(TK_IDENT);
        if (find_lvar(tok)) error_at(tok->str, "すでに定義された変数です\n");
        if (consume("[")) {
            int array_size = expect_number() * calc_size(ty->ty);
            ty = create_type(TP_ARRAY, ty);
            ty->array_size = array_size;
            expect("]");
        }
        LVar *lvar = create_lvar(tok->str, tok->len, sum_offset(locals) + calc_offset(ty), ty);
        push_vector(locals, lvar);
        return new_node_lvar(lvar);
    }
    Token *tok = consume_ident();
    if (tok) {
        if (consume("(")) {  // 関数呼び出し
            Node *node = new_node_func_call(tok->str, tok->len);
            while (true) {
                if (consume(")")) break;
                push_vector(node->args, equality());
                if (consume(")")) break;
                expect(",");
            }
            return node;
        } else {  // 左辺値
            LVar *lvar = find_lvar(tok);
            if (!lvar) error_at(tok->str, "存在しない変数です\n");
            return new_node_lvar(lvar);
        }
    }
    return new_node_num(expect_number());
}