#include "9cc.h"

bool at_eof(){
    return token->kind == TK_EOF;
}

bool consume(char *op){
    if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_kind_of(TokenKind kind){
    if(token->kind != kind) return NULL;
    Token *tok = token;
    token = token->next;
    return tok;
}

Token *consume_ident(){
    if(token->kind != TK_IDENT) return NULL;
    Token *tok = token;
    token = token->next;
    return tok;   
}

void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    error("'%s'ではありません", op);
  token = token->next;
}

int expect_number(){
    if(token->kind != TK_NUM){
        error_at(token->str, "数ではありません");
    }
    int ret = token->val;
    token = token->next;
    return ret;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

LVar *find_lvar(Token *tok){
    for(LVar *var = locals; var; var = var->next){
        if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)) return var;
    }
    return NULL;
}

void *program(){
    locals = calloc(1, sizeof(LVar));
    locals->len = 0;
    locals->offset = 0;
    int i = 0;
    while(!at_eof()){
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *stmt(){
    if(consume_kind_of(TK_RETURN)){
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
        return node;
    }
    Node *node = expr();
    expect(";");
    return node;
}

Node *expr(){
    return assign();
}

Node *assign(){
    Node *node = equality();
    if(consume("=")){
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality(){
    Node *node = relational();
    while(true){
        if(consume("==")){
            node = new_node(ND_EQ, node, relational());
        }else if(consume("!=")){
            node = new_node(ND_NE, node, relational());
        }else{
            return node;
        }
    }
}

Node *relational(){
    Node *node = add();
    while(true){
        if(consume("<")){
            node = new_node(ND_LT, node, add());
        }else if(consume("<=")){
            node = new_node(ND_LE, node, add());
        }else if(consume(">")){
            node = new_node(ND_LT, add(), node);
        }else if(consume(">=")){
            node = new_node(ND_LE, add(), node);
        }else{
            return node;
        }
    }
}

Node *add(){
    Node *node = mul();
    while(true){
        if(consume("+")){
            node = new_node(ND_ADD, node, mul());
        }else if(consume("-")){
            node = new_node(ND_SUB, node, mul());
        }else{
            return node;
        }
    }
}

Node *mul(){
    Node *node = unray();
    while(true){
        if(consume("*")){
            node = new_node(ND_MUL, node, unray());
        }else if(consume("/")){
            node = new_node(ND_DIV, node, unray());
        }else{
            return node;
        }
    }
}

Node *unray(){
    if(consume("-")){
        Node *node = primary();
        node->val = -node->val;
        return node;
    }else if(consume("+")){
        Node *node = primary();
        return node;
    }else{
        Node *node = primary();
    }
}

Node *primary(){
    if(consume("(")){
        Node *node = expr();
        expect(")");
        return node;
    }
    Token *tok = consume_ident();
    if(tok){
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        LVar *lvar = find_lvar(tok);
        if(lvar){
            node->offset = lvar->offset;
        }else{
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->offset = locals->offset+8;
            node->offset = lvar->offset;
            lvar->len = tok->len;
            locals = lvar;
        }
        return node;
    }
    return new_node_num(expect_number());
}