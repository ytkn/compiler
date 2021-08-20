#include <stdlib.h>
#include <stdio.h>
#include "vector.h"

void push_vector(Vector *v, void *val) {
    if(v->size >= v->reserved) {
        v->reserved *= 2;
        v->data = (void**)realloc(v->data, sizeof(void *) * v->reserved);
    }
    v->data[v->size] = val;
    v->size++;
}

Vector *create_vector(){
    Vector *v = (Vector*)calloc(1, sizeof(Vector));
    v->data = (void**)malloc(sizeof(void *) * 1);
    v->size = 0;
    v->reserved = 1;
}
