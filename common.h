#ifndef COMMON_H
#define COMMON_H
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>
#include<ncurses.h>

#define PORT 3082
#define BUFSZ 1024
#define BACKLOG 10
#define DBSZ 10
#define USERFND 1
#define USERADDED 1
#define LOGINSUC 1
#define USERNOTFND 0
#define VECTORRES 4
#define ERR -1

#define swap(x,y) \
 do{                   \
    typeof(x) temp = y; \
    y = x;              \
    x = temp;           \
 }while(0)
void err_exit(char* msg){
    printf("%s\n",msg);
    exit(EXIT_FAILURE);
}

#endif