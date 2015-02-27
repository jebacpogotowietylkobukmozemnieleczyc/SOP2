#ifndef DATA_H
#define DATA_H


typedef void(*Fptr)();

struct Message{
long type;
int action;
int pid;
char nick[16];
char text[1024];
};


struct User{
    int pid;
    char nick[16];
    char password[16];
};

struct Group{
    int id;
    char name[16];
    int * members;
    int count_members;
};

typedef struct Message Msg;

typedef struct User User;
typedef struct Group Group;
#endif // DATA_H

