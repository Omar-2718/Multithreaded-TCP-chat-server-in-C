#include"vector.h"
#include"user.h"
int remove_user_session(vector *session_users,int fd){
    int idx = -1;
    for(int i=0;i<session_users->sz;i++){
       if(((struct user*)get(session_users,i))->fd == fd){
            idx = i;
            break;
       }
    }
    if(idx == -1)return idx;

    size_t ws = sizeof(struct user);
    struct user tmp;
    memcpy(&tmp, session_users->arr + idx * ws, ws);
    memcpy(session_users->arr + idx * ws,
        session_users->arr + (session_users->sz - 1) * ws, ws);
    memcpy(session_users->arr + (session_users->sz - 1) * ws, &tmp, ws);
    pop_back(session_users);
    return 1;
}
void add_user_session(vector *session_users,struct user* cur_user){
    push_back(session_users,cur_user);
}
char* get_user_name_from_fd(vector *session_users,int fd){
    for(int i=0;i<session_users->sz;i++){
        if(((struct user*)get(session_users,i))->fd == fd){
            return ((struct user*)get(session_users,i))->username;
        }
    }
    return NULL;
}