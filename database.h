#include"common.h"
#include"user.h"
extern struct user* users[DBSZ];
extern int sz;
struct user* create_user(char* name,char* pass,int clientSocketFD);
int user_name_exist(char* name,char* pass);
int add_user(char* name,char *pass,int clientSockeFD);
int log_in(char*name,char* pass);