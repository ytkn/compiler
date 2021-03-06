#include "9cc.h"

int calc_size(TypeKind ty) {
    switch (ty) {
        case TP_CHAR:
            return 1;
        case TP_BOOL:
            return 1;
        case TP_INT:
            return 4;
        case TP_PTR:
            return 8;
    }
}

int sum_offset(Vector *locals) {
    int sum = 0;
    for (int i = 0; i < locals->size; i++) {
        LVar *lvar = locals->data[i];
        if (lvar->ty->ty == TP_ARRAY) {
            sum += lvar->ty->array_size * calc_size(lvar->ty->ptr_to->ty);
        } else {
            sum += 8;
        }
    }
    return sum;
}

int max(int a, int b){
    if(a > b) return a;
    return b;
}

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}