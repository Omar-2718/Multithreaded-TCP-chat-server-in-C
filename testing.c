#include<stdio.h>
#include<pthread.h>
#include<time.h>
#include<malloc.h>
#include<stdlib.h> 
void* foo(void* arg){
    pthread_t id = pthread_self();
    printf("Hello from thread with arg %d with id %d\n",arg,(unsigned int)id);
    if(arg == 0){
        pthread_exit("u died");
    }
    // ret will point to string literal which exist for the lifetime of the program
    return "hello";
}
long long int is_prime(long long int n){
    if(n == 1)return 0;
    for(long long int i=2;i*i<=n;i++){
        if(n%i == 0){
            return 0;
        }
    }
    return 1;
}
// n can be at most 1e6
void* calc_prime(void* n){
    pthread_t id = pthread_self();
    printf("Hello from thread with arg %lld with id %d\n",*(long long int*)n,(unsigned int)id);
    // clock() measure cpu time which is an int
    clock_t start = clock();
    int *cnt = malloc(sizeof(int));
    *cnt=0;
    for(int i=1;i<=*(long long int*)n;i++){
        *cnt += is_prime(i);
    }
    clock_t end = clock();
    double cpu_time = (double)(end-start)/CLOCKS_PER_SEC;
    printf("thread with arg %lld with id %d finshed in %f seconds\n",*(long long int*)n,(unsigned int)id,cpu_time);
    return cnt;
}
int main(int argc,char** argv){
    int* ret1,*ret2,*ret3;

    long long int *p1,*p2,*p3;

    p1 = malloc(sizeof(long long int));
    p2 = malloc(sizeof(long long int));
    p3 = malloc(sizeof(long long int));

    *p1 = atoll(argv[1]);
    *p2 = atoll(argv[2]);
    *p3 = atoll(argv[3]);


    pthread_t thread1,thread2,thread3;
    pthread_create(&thread1,NULL,calc_prime,p1);   
    pthread_create(&thread2,NULL,calc_prime,p2);   
    pthread_create(&thread3,NULL,calc_prime,p3);   
    //pthread_cancel(thread); i can cancle
    // each one of them will run but i wait for first one to join
    pthread_join(thread1,&ret1);
    printf("return is %d\n",*ret1);
    pthread_join(thread2,&ret2);
    printf("return is %d\n",*ret2);
    pthread_join(thread3,&ret3);
    printf("return is %d\n",*ret3);

    free(ret1);
    free(ret2);
    free(ret3);
    free(p1);
    free(p2);
    free(p3);
    return 0;
}
