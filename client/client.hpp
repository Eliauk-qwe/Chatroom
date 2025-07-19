#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include "../Message.hpp"
#include "../StickyPacket.hpp"
#include <fcntl.h>
#include <sys/sendfile.h>
#include <thread>


#define SIGNUP 1
#define LOGIN 2
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
#define GROUP_LIST 24
#define GROUP_CREAT 25
#define GROUP_ADD 26
#define GROUP_APPLY_AGREE 27
#define GROUP_APPLY_REFUSE 28
#define CHECK_GROUP_APPLY 29









using namespace std;


extern StickyPacket socket_fd;
extern string log_uid;
extern sockaddr_in client_addr;

void sign_up();
int  log_in();
void pass_find();
void user_dele();
void unread_msg();
void user_menu();
void friend_menu();
void friend_apply();
void friend_del();
void friend_list();
void friend_quit();
void friend_quit_menu();
void friend_quit_list();
void new_friend();
void friend_back();
void friend_apply_agree();
void friend_apply_refuse();
void friend_chat();
void send_file(string uid,string friend_uid,StickyPacket f_socket,int flag);
void recv_file(string uid,string friend_uid,StickyPacket f_socket,int flag);
void check_friend_apply();
void group_menu();
void group_list();
void group_creat();
void group_add();
void group_apply_menu();
void group_apply_agree();
void group_apply_refuse();
void check_group_apply();






