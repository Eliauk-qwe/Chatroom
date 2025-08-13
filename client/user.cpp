#include "client.h"

void user_menu(){
    string opt;
    while(1){
        printf("=============用户界面===========\n");
        printf("选项：\n[1]好友\n[2]群聊\n[3]通知类消息\n[4]用户注销\n[4]用户退出\n");
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
        
        case 4:
            user_quit();
            exit(0);

        case 3:
            notice();
            break;

        case 5:
            exit(0);
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
   
    Message msg(log_uid,INFORM);
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
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
    }
    return;
}