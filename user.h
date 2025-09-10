#ifndef USER_H
#define USER_H
#include"common.h"
struct user{
    char *username;
    char *password;
    int fd;
};
#endif