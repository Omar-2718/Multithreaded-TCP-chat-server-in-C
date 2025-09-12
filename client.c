#include "common.h"

char ip[BUFSIZ];
int socketFD = -1;
void get_ip(char buf[]){
    struct in_addr tmp;
    printf("Enter ip adress or local for local ip : ");
    if(scanf("%s",buf) != 1){
        err_exit("Invalid ip adress\n");
    }
    if(strcmp(buf,"local") == 0){
        strcpy(buf,"127.0.0.1");
        return;
    }
    if (inet_pton(AF_INET, buf, &tmp) <= 0) err_exit("Invalid ip adress");
}
WINDOW* users_border,*main_chat_border,*user_chat_border;
WINDOW* users,*main_chat,*user_chat;
void err_exit_wn(char* msg){
    delwin(users);
    delwin(main_chat);
    delwin(user_chat);
    delwin(users_border);
    delwin(main_chat_border);
    delwin(user_chat_border);
    endwin();
    printf("%s\n",msg);
    if(socketFD != -1)
    close(socketFD);
    exit(EXIT_FAILURE);
}
void exit_program(char* msg){
    delwin(users);
    delwin(main_chat);
    delwin(user_chat);
    delwin(users_border);
    delwin(main_chat_border);
    delwin(user_chat_border);
    endwin();
    close(socketFD);
    if(msg != NULL){
        printf("%s",msg);
    }
    exit(EXIT_SUCCESS);
}
void write_main_chat(char buf[]){
    char *line = strtok(buf,"\n");
    while (line)
    {   
        int cur_i,cur_j;
        getyx(main_chat,cur_i,cur_j);
        if(cur_i >= getmaxy(main_chat)){
            wscrl(main_chat,1);
        }
        wprintw(main_chat,"%s\n", line);
        wrefresh(main_chat);
        line = strtok(NULL,"\n");
    }
}
void update_current_users(char* name){
    char* token = strtok(name," ");
    int cur_i,cur_j;
    wprintw(users,"Active Users : \n");
    while (token)
    {
        getyx(users,cur_i,cur_j);
        wattron(users,COLOR_PAIR(1));
        wprintw(users,"%s\n",token);
        wattroff(users,COLOR_PAIR(1));
        // continue from last left \0 untill next space
        token = strtok(NULL," ");
    }
    wrefresh(users);
    // wprintw(users,)
}
void clear_current_users(){
    wclear(users);
    wrefresh(users);
}
void clear_main_chat(){
    wclear(main_chat);
    wrefresh(main_chat);
}
void *incoming_msg(void *arg) {
    char buf[BUFSIZ];
    memset(buf,0,sizeof buf);
    while (1) {
        int res = recv_msg(socketFD, buf, BUFSIZ - 1);
        if (res <= 0) {
            if(buf[0] != 0){
                err_exit_wn(buf);
            }
            err_exit_wn("Connection lost with the server.\n");
        }
        buf[res] = '\0';
        char name[16];
        if(sscanf(buf,"%15s",name) >= 1){
            if(strcmp("user",name) == 0){
                clear_current_users();
                char* msgp = malloc(sizeof(char) * res);
                memcpy(msgp,buf + 5*sizeof(char),res-5);
                update_current_users(msgp);
            }
            else{
                write_main_chat(buf);
            }
        }
        
    }
    return NULL;
}
int connect_server(){
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1) err_exit_wn("Couldn't connect socket");

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip, &address.sin_addr) <= 0) err_exit_wn("not a valid ip adress");

    int res = connect(socketFD,&address,sizeof(address));
    if(res == 0){
        printf("Connection successful\n");
    }else{
        err_exit_wn("Couldn't establish connection\n");
    }

    pthread_t thread;
    
    if (pthread_create(&thread, NULL, incoming_msg, NULL) != 0) {
        err_exit_wn("Couldn't create thread");
    }
    pthread_detach(thread);

}
void chat_screen(){
    int rows,columns;
    getmaxyx(stdscr,rows,columns);
    users_border = newwin(rows,columns*0.2,0,0);
    main_chat_border = newwin(rows*0.8,columns*0.8,0,0.2*columns);
    user_chat_border = newwin(rows*0.2,columns*0.8,rows*0.8,columns*0.2);

    users = derwin(users_border,rows-2,(columns*0.2)-2,1,1);
    main_chat = derwin(main_chat_border,(rows*0.8)-2,(columns*0.8)-2,1,1);
    user_chat = derwin(user_chat_border,(rows*0.2)-2,(columns*0.8)-2,1,1);
    
    box(users_border,0,0);
    box(main_chat_border,0,0);
    box(user_chat_border,0,0);
    wrefresh(users_border);
    wrefresh(main_chat_border);
    wrefresh(user_chat_border);

    scrollok(main_chat, TRUE);

    init_pair(1,COLOR_GREEN,COLOR_BLACK);
    init_pair(2,COLOR_RED,COLOR_BLACK);

    wattron(users,COLOR_PAIR(1));
    wprintw(users,"Active users window\n");
    wattroff(users,COLOR_PAIR(1));

    wattron(main_chat,COLOR_PAIR(2));
    wprintw(main_chat,"Type clear() to clear chat\nType exit() to exit program\n\n");
    wattroff(main_chat,COLOR_PAIR(2));

    wattron(main_chat,COLOR_PAIR(1));
    wprintw(main_chat,"To login type \"login <username> <password>\"\n");
    wattroff(main_chat,COLOR_PAIR(1));

    wattron(main_chat,COLOR_PAIR(1));
    wprintw(main_chat,"To signup type \"signup <username> <password>\"\n");
    wattroff(main_chat,COLOR_PAIR(1));

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
        wprintw(user_chat,"You : ");
        wrefresh(user_chat);
        if (strcmp(buf, "") == 0) {
            continue;
        }
        if (strcmp(buf, "exit()") == 0) {
            wprintw(user_chat,"Exiting..\n");
            wrefresh(user_chat);
            exit_program("Exit Succesful\n");
            break;
        }
        if (strcmp(buf, "clear()") == 0) {
            clear_main_chat();
            continue;
        }

        if (send_msg(socketFD, buf) <= 0) {
            err_exit_wn("Error sending data");
            break;
        }
    }
}

int main() {
    get_ip(ip);
    get_port();
    initscr();
    
    start_color();
    connect_server();
    run_chat();
    return 0;
}

