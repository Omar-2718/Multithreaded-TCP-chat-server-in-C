#include "common.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
WINDOW* users,*main_chat,*user_chat;
void err_exit_wn(char* msg){

    delwin(users);
    delwin(main_chat);
    delwin(user_chat);

    endwin();
    printf("%s\n",msg);
    exit(EXIT_FAILURE);
}

int socketFD;
int n,m;
void write_main_chat(char buf[]){
    char *line = strtok(buf,"\n");
    while (line)
    {   
        int cur_i,cur_j;
        getyx(main_chat,cur_i,cur_j);
        if(cur_i + 1 >= getmaxy(main_chat)){
            wscrl(main_chat,1);
        }
        wmove(main_chat,cur_i,1);
        wprintw(main_chat,"%s\n", line);
        wrefresh(main_chat);
        line = strtok(NULL,"\n");
    }

}
void update_current_users(char* name){

}
void clear_current_users(){

}
void *incoming_msg(void *arg) {
    char buf[BUFSIZ];
    memset(buf,0,sizeof buf);
    while (1) {
        int res = recv(socketFD, buf, BUFSIZ - 1, 0);
        if (res <= 0) {
            if(buf[0] != 0){
                err_exit_wn(buf);
            }
            err_exit_wn("Connection lost with the server.\n");
        }
        buf[res] = '\0';
        char name[16];
        char msg[BUFSIZ];
        if(sscanf(buf,"%15s %s",name,msg) != 2){
            if(strcmp("user",name) == 0){
                update_current_users(name);
            }
        }
        write_main_chat(buf);
        
    }
    return NULL;
}
int connect_server(){
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1) err_exit("couldnt connect socket");

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    char *ip = "127.0.0.1";
    if (inet_pton(AF_INET, ip, &address.sin_addr) <= 0) err_exit("inet_pton");

    int res = connect(socketFD,&address,sizeof(address));
    if(res == 0){
        printf("connection successful\n");
    }else{
        printf("couldnt connect error %d\n",res);
        return -1;
    }

    pthread_t thread;
    
    if (pthread_create(&thread, NULL, incoming_msg, NULL) != 0) {
        err_exit("couldnt create thread");
    }
    pthread_detach(thread);

}
void chat_screen(){
    getmaxyx(stdscr,n,m);
    users = newwin(n,m*0.2,0,0);
    main_chat = newwin(n*0.8,m*0.8,0,0.2*m);
    user_chat = newwin(n*0.2,m*0.8,n*0.8,m*0.2);
    wmove(users,1,1);
    wmove(main_chat,1,1);
    wmove(user_chat,1,1);
    box(users,0,0);
    box(main_chat,0,0);
    box(user_chat,0,0);
    scrollok(main_chat, TRUE);

    init_pair(1,COLOR_WHITE,COLOR_RED);
    wattron(users,COLOR_PAIR(1));
    mvwprintw(users,1,1,"Active users %d %d\n",n,m);
    wattroff(users,COLOR_PAIR(1));

    wrefresh(users);
    wrefresh(main_chat);
    wrefresh(user_chat);
}
void run_chat(){
    chat_screen();
    while (1) {
        char buf[BUFSIZ];
        int res = wgetnstr(user_chat,buf,BUFSIZ);
        wclear(user_chat);
        wmove(user_chat,1,1);
        wprintw(user_chat,"You : ");
        box(user_chat,0,0);
        wrefresh(user_chat);
        if (strcmp(buf, "exit()") == 0) {
            wprintw(user_chat,"Exiting..\n");
            wrefresh(user_chat);
            close(socketFD);
            break;
        }

        if (send(socketFD, buf, strlen(buf), 0) <= 0) {
            close(socketFD);
            err_exit_wn("error sending data");
            break;
        }
    }
    close(socketFD);
}

int main() {
    initscr();
    start_color();
    connect_server();
    run_chat();
    return 0;
}

