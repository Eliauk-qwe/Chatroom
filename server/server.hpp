#ifndef _SEVER_H_
#define _SEVER_H_
#include <iostream>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "../ThreadPool.hpp"
#include <fcntl.h>
#include <string>
#include <unordered_set>
#include <vector>
#include "../Redis.hpp"
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <thread>
#include <mutex>
#include "../StickyPacket.hpp"
#include "../Message.hpp"




//class StickyPacket;


#define SIGNUP 1
#define LOGIN  2
#define QUESTION_GET 3
#define ANSWER_GET 4
#define PASS_GET 5
#define USER_DEL 6
#define UNREAD_MSG 7
#define FRIEND_ADD 8
#define FRIEND_DEL 9
#define FRIEND_LIST 10
#define FRIEND_QUIT 11
#define FRIEND_QUIT_LIST 12
#define FRIEND_BACK 13
#define FRIEND_APPLY_AGREE 14
#define FRIEND_APPLY_REFUSE 15
#define FRIEND_CHAT 16
#define FRIEND_CHAT_DAILY 17
#define QUIT_CHAT 18
#define FRIEND_SEND_FILE 19
#define FRIEND_RECV_FILE 20
#define PASSFIND 21
#define NOTICE 22
#define CHECK_FRIEND_APPLY 23
#define GROUP_CREAT 24
#define GROUP_LIST 25
#define GROUP_ADD 26
#define GROUP_APPLY_AGREE 27
#define GROUP_APPLY_REFUSE 28
#define CHECK_GROUP_APPLY 29










#define RED "\033[31m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define RESET "\033[0m"
#define WIDEWHITE "\033[1;37m"


void sign_up(StickyPacket socket,Message &msg);
void log_in(StickyPacket socket,Message &msg);
void user_del(StickyPacket socket,Message &msg);
void unread_msg(StickyPacket socket,Message &msg);
void friend_add(StickyPacket socket,Message &msg);
void friend_del(StickyPacket socket,Message &msg);
void friend_list(StickyPacket socket,Message &msg);
void friend_quit(StickyPacket socket,Message &msg);
void friend_quit_list(StickyPacket socket,Message &msg);
void friend_back(StickyPacket socket,Message &msg);
void friend_apply_agree(StickyPacket socket,Message &msg);
void friend_apply_refuse(StickyPacket socket,Message &msg);
void friend_chat(StickyPacket socket,Message &msg);
void friend_chat_daily(StickyPacket socket,Message &msg);
void quit_chat(StickyPacket socket,Message &msg);
void recv_sendfile(StickyPacket socket,Message &msg);
void friend_send_file(StickyPacket socket,Message &msg);
void friend_recv_file(StickyPacket socket,Message &msg);
void send_sendfile(StickyPacket socket,Message &msg);
void passfind(StickyPacket socket,Message &msg);
void check_friend_apply(StickyPacket socket,Message &msg);
void group_creat(StickyPacket socket,Message &msg);
void group_list(StickyPacket socket,Message &msg);
void group_add(StickyPacket socket,Message &msg);
void group_apply_agree(StickyPacket socket,Message &msg);
void group_apply_refuse(StickyPacket socket,Message &msg);
void check_group_apply(StickyPacket socket,Message &msg);









extern Redis redis;
extern int user_uid;
unordered_set<string> online_users;


using namespace std;

#define LISTEN_NUM 150
#define MAX_EVENTS 1024


class User{
public:
   string UID, Name, Pass, Question, Answer,Phone;
   
   User(string name,string uid,string pass,string question,string answer,string phone ){
       this->Answer=answer;
       this->Name=name;
       this->Pass=pass;
       this->Phone=phone;
       this->Question=question;
       this->UID=uid;
   }

private:
   mutex user_mutex;
   
};


class MessageTrans
{
public:
    void translation(StickyPacket socket,const std::string &cmd){
        Message msg;
        msg.Json_to_s(cmd);
        switch (msg.flag)
        {
        case SIGNUP:
            sign_up(socket,msg);
            break;
        case LOGIN:
            log_in(socket,msg);
            break;
        case PASSFIND:
            passfind(socket,msg);
            break;
        case USER_DEL:
            user_del(socket,msg);
            break;
        case UNREAD_MSG:
            unread_msg(socket,msg);
            break;
        case FRIEND_ADD:
            friend_add(socket,msg);
            break;
        case FRIEND_DEL:
            friend_del(socket,msg);
            break;
        case FRIEND_LIST:
            friend_list(socket,msg);
            break;
        case FRIEND_QUIT:
            friend_quit(socket,msg);
            break;
        case FRIEND_QUIT_LIST:
            friend_quit_list(socket,msg);
            break;
        case FRIEND_BACK:
            friend_back(socket,msg);
            break;
        case FRIEND_APPLY_AGREE:
            friend_apply_agree(socket,msg);
            break;
        case FRIEND_APPLY_REFUSE:
            friend_apply_refuse(socket,msg);
            break;
        case FRIEND_CHAT:
            friend_chat(socket,msg);
            break;
        case FRIEND_CHAT_DAILY:
            friend_chat_daily(socket,msg);
            break;
        case QUIT_CHAT:
            quit_chat(socket,msg);
            break;
        case FRIEND_SEND_FILE:
            friend_send_file(socket,msg);
            break;
        case FRIEND_RECV_FILE:
            friend_recv_file(socket,msg);
            break;
        case GROUP_CREAT:
            group_creat(socket,msg);
            break;
        case GROUP_LIST:
            group_list(socket,msg);
            break;
        case GROUP_ADD:
            group_add(socket,msg);
            break;
        case GROUP_APPLY_AGREE:
            group_apply_agree(socket,msg);
            break;
        case GROUP_APPLY_REFUSE:
            group_apply_refuse(socket,msg);
            break;
        case CHECK_FRIEND_APPLY:
            check_friend_apply(socket,msg);
            break;
        case CHECK_GROUP_APPLY:
            check_group_apply(socket,msg);
            break;
        
        default:
            break;
        }

    }
};


#endif