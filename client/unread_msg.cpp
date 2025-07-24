#include "client.hpp"
void unread_msg_menu(){
    string opt;
    while(1){
        printf("----------------未读消息界面--------------\n");
        printf("选项：\n[1]好友申请\n[2]删除好友\n[3]好友列表\n[4]屏蔽好友\n[5]私聊\n[6]新的朋友\n[7]返回\n");
        printf("请输入你的选择：\n");
        getline(cin,opt);
    }
}