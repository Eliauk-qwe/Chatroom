#include "client.hpp"

void user_menu(){
    string opt;
    while(1){
        printf("=============用户界面===========\n");
        printf("选项：\n[1]好友\n[2]群聊\n[3]用户注销\n[4]通知类消息\n[5]退出登录\n");
        printf("请输入你的选择：\n");
        getline(cin,opt);
        printf("===============================\n");

        if (isNotNumber(opt))
        {
            std::cout << "不是数字" << std::endl;
            continue;
        }

        switch (stoi(opt))
        {
        case 1:
            friend_menu();
            break;
        
        case 2:
            group_menu();
            break;
        
        case 3:
            user_quit();
            break;

        case 4:
            notice();
            break;

        case 5:
            client_quit(socket_fd.getfd());
            break;
        
        default:
            printf("请输入正确选项\n");
            break;
        }
    }
}

void user_quit(){
    Message msg(log_uid,USER_QUIT);
    socket_fd.mysend(msg.S_to_json());
    string recv =socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    if(recv=="ok"){
        printf("你已成功注销该账户\n");
        return;
    }
    return;
}

void  notice(){
    //printf("以下是你的所有通知消息:\n");
    Message msg(log_uid,INFORM);
    socket_fd.mysend(msg.S_to_json());
    string recv =socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    /*if(recv=="0"){
        printf("你还没有未读的通知消息\n");
        return;
    }*/
    while(recv !="over"){
        cout<<recv<<endl;
        recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
    }
    return;
}