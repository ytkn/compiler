#include "9cc.h"
#include "vector.h"

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

int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    }
    int ret = token->val;
    token = token->next;
    return ret;
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

LVar *create_lvar(char *name, int len, int offset) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->name = name;
    lvar->len = len;
    lvar->offset = offset;
    return lvar;
}

void *program() {
    prog = calloc(1, sizeof(Program));
    prog->funcs = create_vector();
    while (!at_eof()) {
        push_vector(prog->funcs, function());
    }
}

Function *function() {
    Token *tok = consume_ident();
    Node *node = new_node_func_def();
    Function *fn = new_func(tok->str, tok->len, node);
    locals = fn->locals;
    params = fn->params;
    expect("(");
    while (true) {
        Token *param = consume_ident();
        if (param) {
            LVar *lvar = calloc(1, sizeof(LVar));
            lvar->name = param->str;
            lvar->len = param->len;
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
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
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
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unray());
        } else {
            return node;
        }
    }
}

Node *unray() {
    if (consume("-")) {
        Node *node = primary();
        node->val = -node->val;
        return node;
    } else if (consume("+")) {
        Node *node = primary();
        return node;
    } else {
        Node *node = primary();
    }
}

Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
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
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_LVAR;
            LVar *lvar = find_lvar(tok);
            if (lvar) {
                node->offset = lvar->offset;
            } else {
                lvar = create_lvar(tok->str, tok->len, (locals->size + 1) * 8);
                node->offset = lvar->offset;
                push_vector(locals, lvar);
            }
            return node;
        }
    }
    return new_node_num(expect_number());
}