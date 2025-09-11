#ifndef VECTOR_H
#define VECTOR_H
#include "common.h"
#define VECTOR_INIT {0,0,NULL}
typedef struct vector{
    int sz;
    int cap;
    // becuase thats just a byte
    unsigned char* arr;
    size_t word_sz;

} vector;

void recap(vector* v){
    v->cap *= 2;
    if(!v->cap)v->cap = 1;
    v->arr = realloc(v->arr,v->word_sz * v->cap);
}
void push_back(vector* v,void* val){
    if(v->sz >= v->cap){
        recap(v);
    }
    memcpy(v->arr + v->word_sz * v->sz,val,v->word_sz);
    v->sz++;
}
void pop_back(vector* v){
    v->sz--;
}
void free_elements(vector*v){
    for(int i=0;i<v->sz;i++){
        void* ptr = (void**)(v->arr + (v->word_sz * i));
        free(ptr);
    }
    free_vector(v);
}
void free_vector(vector* v){
    if(v->arr == NULL)return;
    free(v->arr);
    v->sz = 0;
    v->cap = 0;
}
void* get(vector* v,int idx){
    return v->arr + idx * v->word_sz;
}
// int main(){
//     vector arr = VECTOR_INIT;
//     arr.word_sz = sizeof(int);
//     push_back(&arr,&(int){10});
//     push_back(&arr,&(int){20});
//     push_back(&arr,&(int){30});
//     push_back(&arr,&(int){40});
//     for(int i=0;i<arr.sz;i++){
//         printf("%d ",*(int*)get(&arr,i));
//     }
//     free_vector(&arr);
// }
#endif