#ifndef _SEVER_H_
#define _SEVER_H_

#include <iostream>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "./ThreadPool.hpp"
#include <fcntl.h>
#include <string>
#include <unordered_set>
#include <vector>
#include "./Redis.hpp"
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <thread>
#include <mutex>
#include "../JSON.hpp"
#include <csignal>
#include "../StickyPacket.hpp"





#define SIGNUP 1
#define LOGIN  2
#define QUESTION_GET 3
#define ANSWER_GET 4
#define PASS_GET 5
#define USER_QUIT 6
#define NOTICE 7
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
#define FRIEND_QUIT_CHAT 18
#define FRIEND_SEND_FILE 19
#define FRIEND_RECV_FILE 20
#define PASSFIND 21
#define CHECK_FRIEND_APPLY 23
#define GROUP_CREAT 25
#define GROUP_LIST 24
#define GROUP_ADD 26
#define GROUP_APPLY_AGREE 27
#define GROUP_APPLY_REFUSE 28
#define CHECK_GROUP_APPLY 29
#define CHECK_GROUP_MEMBERS 30
#define GROUP_QUIT 31
#define OWNER_ADD_MANAGERS 32
#define OWNER_DEL_MANAGERS 33
#define OWNER_QUIT_GROUP 34
#define ALL_MANAGERS_DEL_MEMBERS 35
#define CHECK_GROUP_MANAGERS 36
#define ACCESS_GROUP 37
#define GROUP_CHAT 38
#define GROUP_DAILY_CHAT 39
#define GROUP_SEND_FILE  40
#define GROUP_RECV_FILE  41
#define GROUP_QUIT_CHAT  42
#define CLIENT_QUIT 43
#define HEART 44
#define INFORM 45
#define INVITE_FRIEND_TO_GROUP 46
#define GROUP_INVITE_AGREE 47
#define GROUP_INVITE_REFUSE 48
#define CHECK_GROUP_INVITE 49
#define IS_FRIEND_CHAT_DAILY 50





#define RED "\033[1;31m"
#define BLUE "\033[34m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define RESET "\033[0m"
#define ZI  "\033[1;35m"
#define PLUSWHITE  "\033[1;37m"
#define QING  "\033[1;36m"





void sign_up(StickyPacket socket,Message &msg);
void log_in(StickyPacket socket,Message &msg);
void user_quit(StickyPacket socket,Message &msg);
void notice(StickyPacket socket,Message &msg);
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
void friend_quit_chat(StickyPacket socket,Message &msg);
int recv_sendfile(StickyPacket socket,Message &msg);
void friend_send_file(StickyPacket socket,Message &msg);
void friend_recv_file(StickyPacket socket,Message &msg);
int send_sendfile(StickyPacket socket,Message &msg);
void passfind(StickyPacket socket,Message &msg);
void check_friend_apply(StickyPacket socket,Message &msg);
void group_creat(StickyPacket socket,Message &msg);
void group_list(StickyPacket socket,Message &msg);
void group_add(StickyPacket socket,Message &msg);
void group_apply_agree(StickyPacket socket,Message &msg);
void group_apply_refuse(StickyPacket socket,Message &msg);
void check_group_apply(StickyPacket socket,Message &msg);
void check_group_members(StickyPacket socket,Message &msg);
void group_quit(StickyPacket socket,Message &msg);
void owner_add_managers(StickyPacket socket,Message &msg);
void owner_del_managers(StickyPacket socket,Message &msg);
void owner_quit_group(StickyPacket socket,Message &msg);
void all_managers_del_members(StickyPacket socket,Message &msg);
void check_group_managers(StickyPacket socket,Message &msg);
void access_group(StickyPacket socket,Message &msg);
void group_chat(StickyPacket socket,Message &msg);
void group_daily_chat(StickyPacket socket,Message &msg,Redis redis);
void group_send_file(StickyPacket socket,Message &msg);
void group_recv_file(StickyPacket socket,Message &msg);
void group_quit_chat(StickyPacket socket,Message &msg);
void client_quit(StickyPacket socket,Message &msg);
void heart(int fd);
void invite_friend_to_group(StickyPacket socket,Message &msg);
void client_dead(int nfd);
void client_lastactive_now(int nfd);
void setnoblock(int fd) ;



 






extern unordered_set<string> online_users;
extern std::unordered_map<int, time_t> last_active_time;
extern std::unordered_map<int, std::string> fd_to_user;
extern std::mutex active_mtx;
extern Redis redis;
extern unordered_map<int, chrono::time_point<chrono::steady_clock>> client_last_active;






using namespace std;

#define LISTEN_NUM 150
#define MAX_EVENTS 1024

class MessageTrans
{
public:
    void translation(StickyPacket socket, const std::string &cmd)
    {
        Message msg;
        msg.Json_to_s(cmd);

        Redis redis;
        switch (msg.flag)
        {
        case SIGNUP:
            sign_up(socket, msg);
            break;
        case LOGIN:
            log_in(socket, msg);
            break;
        case PASSFIND:
            passfind(socket, msg);
            break;
        case USER_QUIT:
            user_quit(socket, msg);
            break;
        case INFORM:
            notice(socket, msg);
            break;
        case FRIEND_ADD:
            friend_add(socket, msg);
            break;
        case FRIEND_DEL:
            friend_del(socket, msg);
            break;
        case FRIEND_LIST:
            friend_list(socket, msg);
            break;
        case FRIEND_QUIT:
            friend_quit(socket, msg);
            break;
        case FRIEND_QUIT_LIST:
            friend_quit_list(socket, msg);
            break;
        case FRIEND_BACK:
            friend_back(socket, msg);
            break;
        case FRIEND_APPLY_AGREE:
            friend_apply_agree(socket, msg);
            break;
        case FRIEND_APPLY_REFUSE:
            friend_apply_refuse(socket, msg);
            break;
        case FRIEND_CHAT:
            friend_chat(socket, msg);
            break;
        case FRIEND_CHAT_DAILY:
            friend_chat_daily(socket, msg);
            break;
        case FRIEND_QUIT_CHAT:
            friend_quit_chat(socket, msg);
            break;
        case FRIEND_SEND_FILE:
            friend_send_file(socket, msg);
            break;
        case FRIEND_RECV_FILE:
            friend_recv_file(socket, msg);
            break;
        case GROUP_CREAT:
            group_creat(socket, msg);
            break;
        case GROUP_LIST:
            group_list(socket, msg);
            break;
        case GROUP_ADD:
            group_add(socket, msg);
            break;
        case GROUP_APPLY_AGREE:
            group_apply_agree(socket, msg);
            break;
        case GROUP_APPLY_REFUSE:
            group_apply_refuse(socket, msg);
            break;
        case CHECK_FRIEND_APPLY:
            check_friend_apply(socket, msg);
            break;
        case CHECK_GROUP_APPLY:
            check_group_apply(socket, msg);
            break;
        case CHECK_GROUP_MEMBERS:
            check_group_members(socket, msg);
            break;
        case OWNER_ADD_MANAGERS:
            owner_add_managers(socket, msg);
            break;
        case OWNER_DEL_MANAGERS:
            owner_del_managers(socket, msg);
            break;
        case OWNER_QUIT_GROUP:
            owner_quit_group(socket, msg);
            break;
        case ALL_MANAGERS_DEL_MEMBERS:
            all_managers_del_members(socket, msg);
            break;
        case CHECK_GROUP_MANAGERS:
            check_group_managers(socket, msg);
            break;
        case ACCESS_GROUP:
            access_group(socket, msg);
            break;
        case GROUP_CHAT:
            group_chat(socket, msg);
            break;
        case GROUP_DAILY_CHAT:
            group_daily_chat(socket, msg, redis);
            break;
        case GROUP_SEND_FILE:
            group_send_file(socket, msg);
            break;
        case GROUP_RECV_FILE:
            group_recv_file(socket, msg);
            break;
        case GROUP_QUIT_CHAT:
            group_quit_chat(socket, msg);
            break;
        case GROUP_QUIT:
            group_quit(socket, msg);
            break;

        case CLIENT_QUIT:
            client_quit(socket, msg);
            break;

        case INVITE_FRIEND_TO_GROUP:
            invite_friend_to_group(socket, msg);
            break;

        case HEART:
            break;

        default:
            break;
        }
    }
};

#endif