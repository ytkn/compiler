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

Type *consume_type() {
    TypeKind kind;
    if (consume_kind_of(TK_INT)) {
        kind = TP_INT;
    } else if (consume_kind_of(TK_CHAR)) {
        kind = TP_CHAR;
    } else {
        return NULL;
    }
    Type *ty = create_type(kind, NULL);
    while (consume("*")) {
        ty = create_type(TP_PTR, ty);
    }
    return ty;
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
    node->is_global = false;
    return node;
}

Node *new_node_global(LVar *lvar) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->name = lvar->name;
    node->name_len = lvar->len;
    node->ty = lvar->ty;
    node->is_global = true;
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

LVar *find_global(Token *tok) {
    for (int i = 0; i < prog->globals->size; i++) {
        LVar *var = prog->globals->data[i];
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) return var;
    }
    return NULL;
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

Function *find_function(Token *tok) {
    for (int i = 0; i < prog->funcs->size; i++) {
        Function *func = prog->funcs->data[i];
        if (func->name_len == tok->len && !memcmp(tok->str, func->name, func->name_len)) return func;
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
    prog->globals = create_vector();
    while (!at_eof()) {
        top_level();
    }
}

void top_level() {
    Type *ty = consume_type();
    if (!ty) parse_error("型ではありません\n");
    Token *tok = consume_ident();
    if (consume("(")) {
        // NOTE: 再帰関数呼び出しに対応するためにfunctionの中でprog->funcsに入れている。
        function(tok);
        return;
    }

    if (consume("[")) {
        int num = expect_number();
        expect("]");
        ty = create_type(TP_ARRAY, ty);
        ty->array_size = num;
        LVar *lvar = create_lvar(tok->str, tok->len, 0, ty);
        push_vector(prog->globals, lvar);
        // parse_error("array in global scope is not implemented\n");
    } else {
        LVar *lvar = create_lvar(tok->str, tok->len, 0, ty);
        push_vector(prog->globals, lvar);
    }
    expect(";");
}

Function *function(Token *tok) {
    Node *node = new_node_func_def();
    Function *fn = new_func(tok->str, tok->len, node);
    locals = fn->locals;
    params = fn->params;
    while (true) {
        Type *ty = consume_type();
        if (ty) {
            Token *param = expect_kind_of(TK_IDENT);
            LVar *lvar = create_lvar(param->str, param->len, 0, ty);
            push_vector(params, lvar);
            if (consume(",")) continue;
        }
        expect(")");
        fn->ty = ty;
        break;
    }
    push_vector(prog->funcs, fn);
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
            node->ty = create_type(TP_BOOL, NULL);
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
            node->ty = create_type(TP_BOOL, NULL);
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
            node->ty = create_type(TP_BOOL, NULL);
        } else if (consume("<=")) {
            node = new_node(ND_LE, node, add());
            node->ty = create_type(TP_BOOL, NULL);
        } else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
            node->ty = create_type(TP_BOOL, NULL);
        } else if (consume(">=")) {
            node = new_node(ND_LE, add(), node);
            node->ty = create_type(TP_BOOL, NULL);
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
            node->ty = node->lhs->ty;  // TODO: ここの型はlhsで決め打つのはだめかもしれない。
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
        node->ty = node->lhs->ty->ptr_to;
        return node;
    } else if (consume("&")) {
        Node *node = new_node(ND_ADDR, unray(), NULL);
        node->ty = create_type(TP_PTR, node->lhs->ty);
        return node;
    } else {
        Node *node = primary();
        if (consume("[")) {  // ここでいいのかなあ
            if (node->kind == ND_LVAR && node->ty->ty == TP_ARRAY) {
                node->ty = create_type(TP_PTR, node->ty->ptr_to);
            }
            if (node->ty->ty != TP_PTR) parse_error("ポインタではありません\n");
            node = new_node(ND_ADD, node, add());
            node->ty = node->lhs->ty;
            node = new_node(ND_DEREF, node, NULL);
            node->ty = node->lhs->ty->ptr_to;
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

    Type *ty = consume_type();
    if (ty) {
        Token *tok = expect_kind_of(TK_IDENT);
        if (find_global(tok) || find_lvar(tok)) error_at(tok->str, "すでに定義された変数です\n");
        if (consume("[")) {
            int array_size = expect_number();
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
            Function *func = find_function(tok);
            if (!func) {
                error_at(tok->str, "定義されていない関数です\n");
            }
            Node *node = new_node_func_call(tok->str, tok->len);
            node->ty = func->ty;
            while (true) {
                if (consume(")")) break;
                push_vector(node->args, equality());
                if (consume(")")) break;
                expect(",");
            }
            return node;
        } else {  // 左辺値
            LVar *lvar = find_global(tok);
            if (lvar) {
                return new_node_global(lvar);
            }
            lvar = find_lvar(tok);
            if (!lvar) error_at(tok->str, "存在しない変数です\n");
            return new_node_lvar(lvar);
        }
    }
    return new_node_num(expect_number());
}