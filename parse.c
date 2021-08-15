#include "9cc.h"

bool consume(char *op){
    if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
        return false;
    }
    token = token->next;
    return true;
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

Node *expr(){
    return equality();
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
        consume(")");
        return node;
    }
    return new_node_num(expect_number());
}