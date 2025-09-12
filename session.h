#include"vector.h"
#include"user.h"
int remove_user_session(vector *session_users,int fd);
void add_user_session(vector *session_users,struct user* cur_user);
char* get_user_name_from_fd(vector *session_users,int fd);