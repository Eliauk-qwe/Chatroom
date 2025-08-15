#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include "../JSON.hpp"
#include "../StickyPacket.hpp"
#include <fcntl.h>
#include <sys/sendfile.h>
#include <thread>
#include <csignal>  // 包含 SIGPIPE, SIG_IGN 和 signal 的声明
#include <cerrno>   // 包含 EPIPE 错误码的声明
#include <limits>
#include <regex>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <ctime>



#define SIGNUP 1
#define LOGIN 2
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
#define GROUP_LIST 24
#define GROUP_CREAT 25
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




using namespace std;


extern StickyPacket socket_fd;
extern string log_uid;
extern sockaddr_in client_addr;

void sign_up();
int  log_in();
void pass_find();
void user_quit();
void notice();
void user_menu();
void friend_menu();
void friend_add();
void friend_del();
int friend_list();
void friend_quit();
void friend_quit_menu();
int friend_quit_list();
void new_friend();
void friend_back();
void friend_apply_agree();
void friend_apply_refuse();
void friend_chat();
int send_file(string uid,string friend_uid,StickyPacket f_socket,int flag,string filepath);
//int recv_file(string uid,StickyPacket f_socket,int flag,string friend_or_group);
int check_friend_apply();
void group_menu();
int group_list();
void group_creat();
void group_add();
void group_apply_menu();
void group_apply_agree();
void group_apply_refuse();
int check_group_apply();
void access_group();
void group_owner_menu(const string groupID,const string group_name);
void group_manager_menu(const string groupID,const string group_name);
void group_common_menu(const string groupID,const string group_name);
void group_chat(const string group_name);
int check_group_members(const string group_name);
void group_quit(const string group_name);
void owner_add_managers(const string group_name);
void owner_del_managers(const string group_name);
void owner_quit_group(const string group_name);
void all_managers_del_members(const string group_name);
int check_group_managers(const string group_name);
void notice_recv_thread(string uid,int noticefd);
void client_quit(int fd,int rfd);
void sfile(string uid,string friend_or_group,int flag,string path);
void fvfile(string uid,string friend_or_group,int flag,string filename,string want_path);
int  friend_recv_file(string uid,StickyPacket f_socket,int flag,string friend_or_group,string filename,string want_path);
void gvfile(string uid,string friend_or_group,int flag,string filename,string want_path,string other_uid);
int group_recv_file(string uid,StickyPacket f_socket,int flag,string friend_or_group,string filename,string want_path,string other_uid);
void heartthread(string uid,int fd);
bool isNotNumber(const std::string& str) ;
void invite_friend_to_group(const string group_name);
void  group_invite_agree();
void  frient_invite_group();
void   group_invite_refuse();
int  check_group_invite();
void notice_recv_thread_1(string uid,int noticefd);
void setnoblock(int fd);






#define RED "\033[1;31m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define RESET "\033[0m"
#define QING  "\033[1;36m"
#define PLUSBLUE  "\033[1;34m"
#define PLUSWHITE  "\033[1;37m"






