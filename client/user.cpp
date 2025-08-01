#include "client.hpp"

void user_menu(){
    string opt;
    while(1){
        printf("--------用户界面--------\n");
        printf("选项：\n[1]好友\n[2]群聊\n[3]注销\n[4]未读消息\n[5]返回\n");
        printf("请输入你的选择：\n");
        getline(cin,opt);
        printf("-----------------------\n");

        switch (stoi(opt))
        {
        case 1:
            friend_menu();
            break;
        
        case 2:
            group_menu();
            break;
        
        /*case 3:
            user_dele();
            break;*/

        case 4:
            unread_msg();
            break;

        case 5:
            return;
            break;
        
        default:
            printf("请输入正确选项\n");
            
        }
    }
}

/*void user_dele(){
    Message msg(log_uid,USER_DEL);
    socket_fd.mysend(msg.S_to_json());
    string recv =socket_fd.client_recv();
    if(recv=="success"){
        printf("注销成功！\n");
        return;
    }
    return;
}*/

void  unread_msg(){
    printf("以下是你的所有未读消息:\n");
    Message msg(log_uid,UNREAD_MSG);
    socket_fd.mysend(msg.S_to_json());
    string recv =socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    while(recv !="over"){
        cout<<recv<<endl;
        recv=socket_fd.client_recv();
    }
    return;
}