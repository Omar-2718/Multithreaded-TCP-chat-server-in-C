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

#define BUFSZ 1024
#define BACKLOG 10
#define DBSZ 10
#define USERFND 1
#define USERADDED 1
#define LOGINSUC 1
#define USERNOTFND 0
#define VECTORRES 4
#define ERR -1
int PORT;
void get_port(){
    printf("Enter port : ");
    if(scanf("%d",&PORT) != 1){
        err_exit("Invalid port\n");
    }
}

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
int send_entire_len(int fd,char* buf,size_t len){
    size_t sent = 0;
    while (sent < len)
    {
        int n = send(fd,buf+sent,len - sent,0);
        if(n <= 0 )return -1;
        sent += n;
    }
    return sent;
    
}
int send_msg(int fd,char* msg){
    uint32_t len = strlen(msg);
    uint32_t nt_len = htonl(len);
    if(send_entire_len(fd,&nt_len,sizeof(nt_len)) != sizeof(nt_len)){
        printf("Error sending msg to %d FD",fd);
        return -1;
    }
    if(send_entire_len(fd,msg,len) != len){
        printf("Error sending msg to %d FD",fd);
        return -1;
    }
    return len;
}
int recv_entire_len(int fd,char* buf,size_t len){
    size_t received = 0;
    while (received < len)
    {
        int n = recv(fd,buf + received,len - received,0);
        if(n <= 0){
            return -1;
        }
        received += n;
    }
    return received;
    
}
int recv_msg(int fd,char* buf,int buf_siz){
    uint32_t nt_len,len;
    if(recv_entire_len(fd,&nt_len,sizeof(nt_len)) != sizeof(nt_len)){
        return -1;
    }
    len = ntohl(nt_len);
    // i want one place for '\0'
    if(len >= BUFSIZ)return -1;
    if(recv_entire_len(fd,buf,len) != len)return -1;
    buf[len] = '\0';
    return len;

}
#endif