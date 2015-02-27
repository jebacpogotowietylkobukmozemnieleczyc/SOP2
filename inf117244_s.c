#include <stdlib.h>
#include <stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
#include"inf117244_sdata.h"
#include <signal.h>
#include <time.h>

Msg msg;
Fptr function[10];
User * users = NULL;
int count_users=0;
int run =1;
Group *groups =NULL;
int count_group = 0;
int sid;

void toexit(){
    run=0;
}

//User funtion

int Adduser(int pid,char * nick){
    if(count_users++==0){
         users = (User*) malloc(count_users*sizeof(User));
    }
    else{
        users = (User*) realloc(users,count_users*sizeof(User));
    }
    users[count_users-1].pid = pid;
    strcpy( users[count_users-1].nick,nick);
    return 1;
}


int Ispid(int pid){
    int i;
    for(i=0;i<count_users;i++){
        if( users[i].pid==pid)return 1;
    }
    return 0;
}

char *  getNick(int pid){
    int i;
    for(i=0;i<count_users;i++){
        if( users[i].pid==pid)return users[i].nick;
    }
    return 0;
}


int Isnick(char * nick){
    int i;
    for(i=0;i<count_users;i++){
        if(strcmp(users[i].nick,nick)==0)return i;
    }
    return -1;
}



int Deleteuser(int pid){
    int i;
    for(i=0;i<count_users;i++){
        if( users[i].pid==pid){
            users[i].pid=users[count_users-1].pid;
            strcpy(users[i].nick,users[--count_users].nick);
            User * newusers = (User*) malloc( count_users*sizeof(User) );
            memcpy(newusers,users,count_users);
            free(users);
            users = newusers;

            return 1;
        }
    }
    return 0;
}


//Group function


int Addmember(int id,int pid){
    if( groups[id].count_members++==0){
         groups[id].members = (int*) malloc( groups[id].count_members*sizeof(int));
    }
    else{
         groups[id].members = (int*) realloc( groups[id].members, groups[id].count_members*sizeof(int) );
    }
    groups[id].members[groups[id].count_members-1] =pid;

    return 1;
}


int Deletemember(int id,int pid){
    int i;
    for(i=0;i<groups[id].count_members;i++){
        if( groups[id].members[i]==pid){
            groups[id].members[i]=groups[id].members[--groups[id].count_members];
            int * newmember = (int*) malloc( groups[id].count_members*sizeof(int) );
            memcpy(newmember,groups[id].members,groups[id].count_members);
            free(groups[id].members);
            groups[id].members = newmember;

            return 1;
        }
    }
    return 0;
}


int Ismember(int id,int pid,char * nick){
    int i;
    for(i=0;i<groups[id].count_members;i++){
        if( groups[id].members[i]==pid || users[pid].nick==nick )return 1;

    }
    return 0;
}



int Isgroup(char * name){
    int i;
    for(i=0;i<count_group;i++){
        if(strcmp(groups[i].name,name)==0)return i;
    }
    return -1;
}


//action funtion

void login(){

    if(Ispid(msg.pid)){
        msg.pid = 3;
        return;
    }
    if(Isnick(msg.nick)!=-1){
        msg.pid = 1;
        return;
    }
    Adduser(msg.pid,msg.nick);
    msg.pid =0;
    printf("Nowy użytkownik ... \n");
}

void showusers(){
    int i;
    for(i=0;i<count_users;i++){
        if(i==0){
            strcpy(msg.text,users[i].nick);
            strcat(msg.text,"\n");
            continue;
        }
        strcat(msg.text,users[i].nick);
        strcat(msg.text,"\n");
    }

    msg.pid =0;
}


void spm(){
    int id = Isnick(msg.nick);
    char text[1024];
    char nick[16];
    strcpy(text,msg.text);
    strcpy(nick,getNick(msg.pid) );
    if(id==-1){
        msg.type = msg.pid;
        msg.pid = 4;
        return;
    }
    time_t t = time(NULL);
    struct tm tm = *localtime( &t );
    sprintf(msg.text,"%s \n %d-%d-%d %d:%d:%d\n %s \n", nick,tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,text);
    printf("Użytkownik %s wysłał prywatną wiadomość do %s ...\n",nick,msg.nick);
    msg.pid =9;
    msg.type=users[id].pid;
    msgsnd(sid,&msg,sizeof(Msg),0);
    msg.pid=0;
}


void sgm(){
    int id = Isgroup(msg.nick);
    char text[1024];
    char nick[16];
    strcpy(text,msg.text);
    strcpy(nick,getNick(msg.pid) );
    if(id==-1){
        msg.type = msg.pid;
        msg.pid = 5;

        return;
    }
    if(!Ismember(id,msg.pid,msg.nick)){
        msg.type = msg.pid;
        msg.pid = 7;
        return;
    }
    time_t t = time(NULL);
    struct tm tm = *localtime( &t );
    sprintf(msg.text,"%s \n %s \n %d-%d-%d %d:%d:%d\n %s \n", msg.nick,nick,tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,text);
    printf("Użytkownik %d wysłał  wiadomość grupową do %s ...\n",msg.pid,msg.nick);
    msg.pid =9;
    int i;
    for(i =0;i<groups[id].count_members;i++){
        msg.type=groups[id].members[i];
        msgsnd(sid,&msg,sizeof(Msg),0);
    }
    msg.pid=0;
}



void showgroups(){
    int i;
    for(i=0;i<count_group;i++){
        if(i==0){
            strcpy(msg.text,groups[i].name);
            strcat(msg.text,"\n");
            continue;
        }
        strcat(msg.text,groups[i].name);
        strcat(msg.text,"\n");
    }

    msg.pid =0;
}


void changenick(){

    if(Isnick(msg.nick)!=-1){
        msg.pid = 1;
        return;
    }


    int i;
    for(i=0;i<count_users;i++){
        if( users[i].pid==msg.pid)break;
    }
    strcpy(users[i].nick,msg.nick);
    msg.pid =0;
    printf("Zmiana nicka ... \n");
}



void joingroup(){
    int id=Isgroup(msg.nick);
    if(id==-1){
        msg.pid = 5;
        return;
    }
    if(Ismember(id,msg.pid,msg.nick)){
        msg.pid = 6;
        return;
    }
    Addmember(id,msg.pid);
    printf("Użytkownik %d dołączył do grupy %s ... \n",msg.pid,groups[id].name);
     msg.pid =0;
}

void disjoingroup(){
    int id=Isgroup(msg.nick);
    if(id==-1){
        msg.pid = 5;
        return;
    }
    if(!Ismember(id,msg.pid,msg.nick)){
        msg.pid = 7;
        return;
    }
    Deletemember(id,msg.pid);
    printf("Użytkownik %d opuścił do grupy %s ... \n",msg.pid,groups[id].name);
     msg.pid =0;
}



void logout(){
    Deleteuser(msg.pid);
    printf("Użytkownik %d usunięty.\n",msg.pid);
     msg.pid =0;
}







int main(void)
{

    //function
    function[0] = &login;
    function[1] = &showusers;
    function[2] = &spm;
    function[3] = &sgm;
    function[4] = &showgroups;
    function[5] = &changenick;
    function[6] = &joingroup;
    function[7] = &disjoingroup;
    function[9] = &logout;

    //czytanie configu

    int fd = open("config",O_RDONLY);
    char  buf ;
    int i=0;
    count_group =0;
    groups = (Group*) malloc(sizeof(Group));
    char group_name[16];
    memset(group_name, 0, 16);

    while(read(fd,&buf,1) > 0)
    {
        if(buf=='\n'){
            printf ("Dodano grupe: %s \n",group_name);
            groups = (Group*)realloc(groups,++count_group*sizeof(Group));
            strcpy(groups[count_group-1].name,group_name);
            groups[count_group-1].id = count_group;
            groups[count_group-1].count_members = 0;
            memset(group_name, 0, 16);
            i=0;

            continue;
        }
        group_name[i]=buf;
        i++;
    }



//serwer

    printf("Start .. \n");
    signal(SIGINT,toexit);

    sid = msgget(12345, 0600 | IPC_CREAT);
    while(run){
        msgrcv(sid,&msg,sizeof(Msg),1,0);
        int pid = msg.pid;
        if(msg.action>=0 && msg.action < 10)function[msg.action]();
        else printf("Błedny status wiadomości \n");       
        msg.type =pid;
        msgsnd(sid,&msg,sizeof(Msg),0);

    }

    msgctl(sid, IPC_RMID, NULL);
    printf("Koniec \n");
    return 0;
}

