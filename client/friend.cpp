#include "client.hpp"

void friend_menu(){
    string opt;
    while(1){
        printf("---------好友界面-------\n");
        printf("选项：\n[1]申请添加好友\n[2]删除好友\n[3]好友列表\n[4]屏蔽好友\n[5]私聊\n[6]新的朋友（添加好友）\n[7]返回\n");
        printf("请输入你的选择：\n");
        getline(cin,opt);

        switch (stoi(opt))
        {
        case 1:
            friend_add();
            break;
        case 2:
            friend_del();
            break;
        case 3:
            friend_list();
            break;
        case 4:
            friend_quit_menu();
            break;
        case 5:
            friend_chat();
            break;
        case 6:
            new_friend();
            break;
        case 7:
            return;
            
        default:
            printf("请输入正确选择\n");
            break;
        }
    }
}

void friend_add(){
    string friend_add_uid,self_intro;
    cout<<"你想添加的好友uid为:"<< endl;
    getline(cin,friend_add_uid);
    cout<<"你的验证信息为（自我介绍）:" << endl;
    getline(cin,self_intro);

    Message msg(log_uid,FRIEND_ADD,friend_add_uid,self_intro);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();

    if (recv == "该用户不存在")
    {
        printf("你想添加的用户不存在\n");
        return;
    }
    /*else if (recv == "friend_exit")
    {
        printf("该用户你已添加，无需添加\n");
        return;
    }*/
    else if (recv == "receive_friend_apply")
    {
        printf("对方已向你发送过好友申请，可到新的朋友界面，实现添加\n");
        return;
    }
    else if (recv == "have_send")
    {
        printf("你曾向对方发送过好友申请，无需再次发送\n");
        return;
    }
    else if (recv == "ok")
    {
        printf("已成功发送添加信息\n");
        return;
    }
    else if (recv == "no")
    {
        printf("不能自己添加自己");
        return;
    }
}


void friend_del(){
    string friend_del_uid;
    cout<<"你想删除的好友uid为:"<< endl;
    getline(cin,friend_del_uid);

    Message msg(log_uid,FRIEND_DEL,friend_del_uid);
    socket_fd.mysend(msg.S_to_json());
    
    string recv=socket_fd.client_recv();
    if(recv=="friend_no_exist"){
        printf("你们本来就没有好友关系\n");
        return;
    }else if(recv=="ok"){
        printf("已成功删除该好友\n");
        return;
    }else if(recv=="no"){
        printf("不能自己删除自己");
        return;
    }
}


void friend_list(){
    Message msg(log_uid,FRIEND_LIST);
    socket_fd.mysend(msg.S_to_json());

    string recv;

    recv=socket_fd.client_recv();
    
    if(recv=="no_exit"){
        printf("你还没有好友\n");
        return;
    }

    while(recv!="over"){
        cout<<recv<<endl;
        recv=socket_fd.client_recv();
    }

    
    printf("以上是你的好友列表\n");
    return;

}


void friend_chat(){
    string friend_chat_uid;
    printf("你想聊天的好友的uid为:\n");
    getline(cin,friend_chat_uid);

    Message msg(log_uid,FRIEND_CHAT,friend_chat_uid);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();
    if(recv=="friend_no_exist"){
        printf("你未添加该用户\n");
        return;
    }else if(recv=="no"){
        printf("该用户未注册\n");
        return;
    }
    else if(recv=="start"){
        string chat_what;
        printf("历史聊天记录为:\n");
        while((chat_what=socket_fd.client_recv()) != "over"){
            cout << chat_what <<endl;
        }
        printf("现在可以开始新的聊天了:\n");
        printf("HELP(如果你想收发文件或退出):\n");
        printf("[1]请输入 <send> 来发送文件\n");
        printf("[2]请输入 <recv> 来接受文件\n");
        printf("[3]输入 <quit> 可退出\n\n");
        
    }
    

    
    while(1){
        string notice;
        getline(cin,notice);
        cout<<"notice:"<<notice<<endl;
        if(notice == "quit"){
            printf("退出聊天\n");
            Message msg(log_uid,FRIEND_QUIT_CHAT,friend_chat_uid);
            socket_fd.mysend(msg.S_to_json());
            string recv=socket_fd.client_recv();
            if(recv=="ok"){
                return;
            }
        }

        if(notice=="send"){
            string filepath;
            printf("请输入你要发送的文件的路径:\n");
            getline(cin, filepath);
            thread thread([uid=log_uid,friend_or_group=friend_chat_uid,path=filepath](){
                sfile(uid,friend_or_group,FRIEND_SEND_FILE,path);
            });
            thread.detach();
            continue;
        }


        if(notice == "recv"){
            printf("你要下载的文件名为：\n");
            string filename;
            getline(cin, filename);

            printf("请输入你想存储的文件的位置（无需以 / 结尾）：\n");
            string want_path;
            getline(cin, want_path);

            thread thread([uid=log_uid,friend_or_group=friend_chat_uid,filename=filename,want_path=want_path](){
                fvfile(uid,friend_or_group,FRIEND_RECV_FILE,filename,want_path);
            });
            thread.detach();
            continue;

        }


        Message msg(log_uid,FRIEND_CHAT_DAILY,friend_chat_uid,notice);
        socket_fd.mysend(msg.S_to_json());
        
        string recv=socket_fd.client_recv();

        if(recv=="del"){
            printf("你已被对方删除\n");
            continue;
        }else if(recv=="quit"){
            printf("你已被对方屏蔽\n");
            continue;
        }
        else if(recv=="ok"){
            continue;
        }
    }

}







