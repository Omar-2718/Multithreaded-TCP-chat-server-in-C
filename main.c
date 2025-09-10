#include"common.h"
#include"user.h"
#include"database.h"
#include "session.h"
#include<pthread.h>
#include<ncurses.h>
#include<time.h>
int serverSocketFD;
struct sockaddr_in serverAdress;
vector session_users = {0,0,NULL,sizeof(struct user)};
void get_time(char buf[]){
    time_t t = time(NULL);
    struct tm *tm_struc;
    tm_struc = localtime(&t);
    strftime(buf,9,"%I:%M %p",tm_struc);
}
void create_server(){
    // create a socket on an ip and port
    serverSocketFD = socket(AF_INET,SOCK_STREAM,0);
    if(serverSocketFD < 0){
        err_exit("Error creating server socket");
    }else{
        printf("Created socket\n");
    }
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(PORT);
    serverAdress.sin_addr.s_addr = INADDR_ANY;

    // optain this ip and port from the operating system (no one can use it other than me)
    // where i can listen or do what i want
    int res = bind(serverSocketFD,&serverAdress,sizeof(struct sockaddr_in));
    if(res == 0){
        printf("Socket was bound successfuly\n");
    }
    else{
        err_exit("Error couldnt bound\n");
    }
    // queue connections up to 10
    int lisRes = listen(serverSocketFD,BACKLOG);
    if(lisRes == 0){
        printf("listen was successful\n");
    }
    else{
        err_exit("Error couldnt listen\n");
    }
    
}
// bytes user msg
// bytes user name


void broadcast(int clientFd,char* msg){
    char* sender_name =  get_user_name_from_fd(&session_users,clientFd);
    for(int i=0;i<session_users.sz;i++){
        struct user *cur = (struct user*)get(&session_users,i);
        // if(cur->fd == clienFd)continue;
        char buf[BUFSIZ*2];
        char tm[9];
        get_time(tm);
        sprintf(buf,"%s <%s> : %s\n",sender_name,tm,msg);
        send_msg(cur->fd,buf);
    }
}

void broadcast_connected_users(){    
    char buf[BUFSIZ*2] = {"user \0"};
    for(int i=0;i<session_users.sz;i++){
        struct user *cur = (struct user*)get(&session_users,i);
        strcat(buf,cur->username);
        if(i+1 == session_users.sz){
            strcat(buf,"\n");
        }
        else 
            strcat(buf," ");
    }   
    for(int i=0;i<session_users.sz;i++){
        struct user *cur = (struct user*)get(&session_users,i);
        send_msg(cur->fd,buf);
    }
}
// LOGIN USERNAME PASSWORD
// SIGNUP USERNAME PASSWORD
void close_connection(int clientSocketFD,char* msg){
    send_msg(clientSocketFD,msg);
    close(clientSocketFD);
}
int clientSocketFD;
int log_user(int clientSocketFD){

    char buf[BUFSIZ];
    int n = recv_msg(clientSocketFD,buf,BUFSIZ-1);
    if(n <= 0){
        close_connection(clientSocketFD,"Wrong format1\n");
        return ERR;
    }
    buf[n] = '\0';
    char tp[16],name[64],pass[64];
    if(sscanf(buf,"%15s %63s %63s",tp,name,pass) != 3){
        close_connection(clientSocketFD,"Wrong format2\n");
        return ERR;
    }
    if(strcmp("login",tp) == 0){
        if(log_in(name,pass) == ERR){
            close_connection(clientSocketFD,"User name not found or wrong password\n");
            return ERR;
        }
        if(strcmp(name,"user") == 0){
            close_connection(clientSocketFD,"user name cant be user\n");
            return ERR;
        }
        add_user_session(&session_users,create_user(name,pass,clientSocketFD));
        return clientSocketFD;
    }
    if(strcmp("signup",tp) == 0){
        if(user_name_exist(name,pass) == USERFND){
            close_connection(clientSocketFD,"User name exist before\n");
            return ERR;
        }
        if(add_user(name,pass,clientSocketFD) == ERR){
            close_connection(clientSocketFD,"Sorry maximum number of users logged\n");
            return ERR;

        }
        if(strcmp(name,"user") == 0){
            close_connection(clientSocketFD,"user name cant be user\n");
            return ERR;
        }
        add_user_session(&session_users,create_user(name,pass,clientSocketFD));

        return clientSocketFD;
    }
    close_connection(clientSocketFD,"Wrong format");
    return ERR;

}
void* chat(void* clientFd){
    if(log_user(*(int*)clientFd) == ERR)return NULL;
    char buf[BUFSIZ];
    broadcast_connected_users();
    while(1){
        int n = recv_msg(*(int*)clientFd,buf,BUFSIZ-1);
        if(n <= 0){
            remove_user_session(&session_users,*(int*)clientFd);
            broadcast_connected_users();
            break;
        }
        buf[n] = '\0';
        broadcast(*(int*)clientFd,buf);
    }
    free(clientFd);
    return NULL;
}
void process_connection(){
    struct sockaddr_in clientAdress;
    int clientAdressSize = sizeof(struct sockaddr_in);
    clientSocketFD = accept(serverSocketFD,&clientAdress,&clientAdressSize);
    if(clientSocketFD < 0){
        // fix later
        err_exit("Error connecting the client socket\n");
    }
    
    pthread_t thread;
    int *fd = malloc(sizeof(int));
    *fd = clientSocketFD;
    pthread_create(&thread,NULL,chat,fd);

}
void show_connected_users(){
    for(int i=0;i<session_users.sz;i++){
        printf("UserName : %s | fileDS : %d | pass : %s\n",((struct user*)get(&session_users,i))->username,
    ((struct user*)get(&session_users,i))->fd,
    ((struct user*)get(&session_users,i))->password);
    }
}
void* tst(void*){
    while (1)
    {
        if(getchar()){
            printf("printing\n");
            show_connected_users();
        }
    }
    return NULL;
}
void run_server(){
    create_server();
    pthread_t thread;
    pthread_create(&thread,NULL,tst,NULL);
    pthread_detach(&thread);
    while (1)
    {
       process_connection();
    }
    
}
int main(){
    // blocking call that will return fd for the connected socket
    run_server();
    // while (1)
    // {
    //     char buf[1024];
    //     recv(clientSocketFD,buf,1024,0);
    //     printf("User 2: %s\n",buf);

    //     char* line = NULL;
    //     size_t len =0;
    //     printf("You : ");
    //     int nread = getline(&line,&len,stdin);
    //     if(nread < 0){
    //         printf("error reading messege\n");
    //         return -1;
    //     }
    //     if(strcmp(line,"exit\n") == 0){
    //         printf("Exiting..\n");
    //         return -1;
    //     }
    //     send(clientSocketFD,line,len,0);
    // }
    free_vector(&session_users);
    close(clientSocketFD);
    shutdown(serverSocketFD,SHUT_RDWR);
    return 0;
}