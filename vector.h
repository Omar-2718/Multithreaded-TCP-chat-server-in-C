#ifndef VECTOR_H
#include<stdlib.h>
#define VECTOR_H
#define VECTOR_INIT {0,0,NULL}
typedef struct vector{
    int sz;
    int cap;
    unsigned char* arr;
    size_t word_sz;

} vector;

void recap(vector* v);
void push_back(vector* v,void* val);
void pop_back(vector* v);
void free_elements(vector*v);
void free_vector(vector* v);
void* get(vector* v,int idx);
#endif