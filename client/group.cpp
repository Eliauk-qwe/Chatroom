#include "client.hpp"
void group_menu(){
    string opt;
    while(1){
        printf("---------------------群聊界面----------------\n");
        printf("选项：\n[1]你的群聊列表\n[2]创建群聊\n[3]申请加群\n[4]群聊申请\n[5]进入群聊\n[6]返回\n");
        printf("请输入你的选择:\n");
        getline(cin,opt);
        printf("--------------------------------------------\n");

        switch (stoi(opt))
        {
        case 1:
            group_list();
            break;
        case 2:
            group_creat();
            break;
        case 3:
            group_add();
            break;
        case 4:
            group_apply_menu();
            break;

        
        default:
            break;
        }

    }
}



void group_list(){
    Message msg(log_uid,GROUP_LIST);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();

    if(recv=="no"){
        printf("你还没有群聊\n");
        return;
    }

    while((recv=socket_fd.client_recv())!="over"){
        cout << recv << endl;
    }
    return;
}

void gruop_creat(){
    printf("输入你想创建的群聊的名字\n");
    string gruop_name;
    getline(cin,gruop_name);

    Message msg(log_uid,GROUP_CREAT,gruop_name);
    socket_fd.mysend(msg.S_to_json());

    string recv;
    recv=socket_fd.client_recv();

    if(recv=="name_have_exit"){
        printf("该名字已存在，请更改名字\n");
        group_creat();
        return;
    }

    if(recv=="ok"){
        printf("群聊创建成功\n");
        return;
    }


}

void group_add(){
    printf("输入你想加入的群聊的名字\n");
    string gruop_name;
    getline(cin,gruop_name);

    printf("你的自我介绍是：\n");
    string self_intro;
    getline(cin,self_intro);

    Message msg(log_uid,GROUP_CREAT,gruop_name,self_intro);
    socket_fd.mysend(msg.S_to_json());

    string recv;
    recv=socket_fd.client_recv();

    if(recv=="name_have_no_exit"){
        printf("不存在这个群聊\n");
        return;
    }

    if(recv=="have_exit"){
        printf("你已经在这个群聊里了，无需申请\n");
        return;
    }

    if(recv=="have_send"){
        printf("你曾经向该群发送过申请，无需再次发送\n");
        return;
    }

    if(recv=="ok"){
        printf("群聊申请发送成功\n");
    }

}




