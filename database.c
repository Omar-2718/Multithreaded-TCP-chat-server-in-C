#include"common.h"
#include"user.h"
struct user* users[DBSZ];
int sz = 0;
struct user* create_user(char* name,char* pass,int clientSocketFD){
    struct user *cur = malloc(sizeof *cur);
    cur->username = strdup(name);
    cur->password = strdup(pass);
    cur->fd = clientSocketFD;
    return cur;
}
int user_name_exist(char* name,char* pass){
    for(int i=0;i<sz;i++){
        if(strcmp(users[i]->username, name) == 0){
            return USERFND;
        }
    }
    return USERNOTFND;
}

int add_user(char* name,char *pass,int clientSockeFD){
    if(user_name_exist(name,pass) == USERFND)return ERR;
    if(sz >= DBSZ)return ERR;
    users[sz++] = create_user(name,pass,clientSockeFD);
    return USERADDED;
}

int log_in(char*name,char* pass){
    for(int i=0;i<sz;i++){
        if(strcmp(users[i]->username,name) == 0){
            if(strcmp(pass,users[i]->password) == 0){
                return LOGINSUC;
            }
            else{
                return ERR;
            }
        }
    }
    return ERR;
}