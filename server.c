#include"common.h"
#include"user.h"
#include"database.h"
#include "session.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int serverSocketFD;
struct sockaddr_in serverAdress;
vector session_users = {0,0,NULL,sizeof(struct user)};
vector all_fd = {0,0,NULL,sizeof(int)};
vector threads = {0,0,NULL,sizeof(pthread_t)};
int running = 1;
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
        printf("Listen was successful\n");
    }
    else{
        err_exit("Error couldnt listen\n");
    }
    
}
// bytes user msg
// bytes user name


void broadcast(int clientFd,char* msg){
    pthread_mutex_lock(&lock); 
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
    pthread_mutex_unlock(&lock); 
}

void broadcast_connected_users(){   
    pthread_mutex_lock(&lock); 
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
    pthread_mutex_unlock(&lock); 
}
// LOGIN USERNAME PASSWORD
// SIGNUP USERNAME PASSWORD
void close_connection(int clientSocketFD,char* msg){
    send_msg(clientSocketFD,msg);
    close(clientSocketFD);
}
int log_user(int clientSocketFD){

    char buf[BUFSIZ];
    int n = recv_msg(clientSocketFD,buf,BUFSIZ-1);
    if(n <= 0){
        close_connection(clientSocketFD,"Wrong format1\n");
        return ERR;
    }
    buf[n] = '\0';
    char tp[16],name[16],pass[16];
    if(sscanf(buf,"%15s %15s %15s",tp,name,pass) != 3){
        close_connection(clientSocketFD,"Wrong format2\n");
        return ERR;
    }
    if(strcmp("login",tp) == 0){
        if(log_in(name,pass) == ERR){
            close_connection(clientSocketFD,"User name not found or wrong password\n");
            return ERR;
        }
        if(strcmp(name,"user") == 0){
            close_connection(clientSocketFD,"User name can't be user\n");
            return ERR;
        }
        pthread_mutex_lock(&lock);
        add_user_session(&session_users,create_user(name,pass,clientSocketFD));
        pthread_mutex_unlock(&lock);
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
            close_connection(clientSocketFD,"User name can't be user\n");
            return ERR;
        }
        
        pthread_mutex_lock(&lock);
        add_user_session(&session_users,create_user(name,pass,clientSocketFD));
        pthread_mutex_unlock(&lock);

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
            pthread_mutex_lock(&lock);
            remove_user_session(&session_users,*(int*)clientFd);
            pthread_mutex_unlock(&lock);

            broadcast_connected_users();
            free(clientFd);
            return NULL;
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
    int clientSocketFD = accept(serverSocketFD,(struct sockaddr*)&clientAdress,&clientAdressSize);
    if(clientSocketFD < 0){
        // err_exit("Error connecting the client socket\n");
        return;
    }
    
    pthread_t thread;    
    int *fd = malloc(sizeof(int));
    *fd = clientSocketFD;
    push_back(&all_fd,fd);
    push_back(&threads,&thread);
    pthread_create(&thread,NULL,chat,fd);

}
void show_connected_users(){
    for(int i=0;i<session_users.sz;i++){
        printf("UserName : %s | fileDS : %d | pass : %s\n",((struct user*)get(&session_users,i))->username,
    ((struct user*)get(&session_users,i))->fd,
    ((struct user*)get(&session_users,i))->password);
    }
}
void exit_program(){
    printf("Exiting\n");
    fflush(stdout);
    // for(int i=0;i<all_fd.sz;i++){
    //     close(*(int*)get(&all_fd,i));
    // }
    for(int i=0;i<threads.sz;i++){
        pthread_t thread = *(pthread_t*)get(&threads,i);
        pthread_join(thread,NULL);
    }

    free_vector(&threads);
    free_vector(&session_users);
    free_vector(&all_fd);
}
void* tst(void*){
    printf("Enter q to quit or w to show connected users\n");
    while (1)
    {   
        int c = getchar();
        if(c == 'q'){
            shutdown(serverSocketFD,SHUT_RDWR);
            running = 0;
            return NULL;
        }
        if(c == 'w'){
            printf("Printing connected users\n");
            show_connected_users();
        }
        
    }
    return NULL;
}
void* run_server_thread(){
    create_server();
    pthread_t thread;
    push_back(&threads,&thread);
    pthread_create(&thread,NULL,tst,NULL);
    while (running)
    {
       process_connection();
    }
    return NULL;    
}
void run_server(){
    pthread_t serv_thread;
    pthread_create(&serv_thread,NULL,run_server_thread,NULL);
    pthread_join(serv_thread,NULL);

}

int main(){
    signal(SIGPIPE, SIG_IGN);
    get_port();
    run_server();
    exit_program();
    return 0;
}