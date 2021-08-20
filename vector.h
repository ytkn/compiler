#ifndef VECTOR_H
#define VECTOR_H

#define get_int(v, i) *((int*)v->data[i]);
#define at(v, i, T) *((T*)v->data[i]);

typedef struct{
  void **data;
  int size;
  int reserved;
} Vector;

void push_vector(Vector *vector, void *val);
Vector *create_vector();

#endif