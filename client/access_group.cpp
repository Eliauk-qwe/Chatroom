#include "client.hpp"
void group_chat_menu(const string group_name){
    string opt;
    while(1){
        cout<<"================"<<group_name<<"=============="<<endl;
        printf("选项：\n[1]群聊聊天\n[2]查看群成员\n[3]退出群聊\n[4]群主添加管理员\n[5]群主删除管理员\n[6]群主解散群聊\n[7]高级权限者踢出用户\n[8]查看群管理员\n[9]返回\n");
        printf("请输入你的选择：\n");
        getline(cin,opt);
        printf("=============================================\n");

        switch (stoi(opt))
        {
        case 1:
            group_chat(group_name);
            break;
        case 2:
            check_group_members(group_name);
            break;
        case 3:
            group_quit(group_name);
            break;
        case 4:
            owner_add_managers(group_name);
            break;
        case 5:
            owner_del_managers(group_name);
            break;
        case 6:
            owner_quit_group(group_name);
            break;
        case 7:
            all_managers_del_members(group_name);
            break;
        case 8:
            check_group_managers(group_name);
            break;
        case 9:
            return;
        default:
            break;
        }
    }
}


void check_group_members(const string group_name){
    Message msg(CHECK_GROUP_MEMBERS,group_name);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    if(recv=="no_group_name"){
        printf("不存在该群\n");
        return;
    }

    while(recv != "over"){
        cout<<recv<<endl;
        recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
    }

    printf("以上是该群的所有群成员\n");
    
}

void group_quit(const string group_name){
    Message msg(log_uid,GROUP_QUIT,group_name);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    if(recv=="no_group_member"){
        printf("你本来就不是该群的成员\n");
        return;
    }

    if(recv=="quit"){
        printf("你是该群群主，不能退出群聊\n");
        return;
    }

    if(recv=="ok"){
        printf("你已成功退出群聊\n");
        return;
    }
}

void owner_add_managers(const string group_name){
    check_group_members(group_name);

    printf("你想设置为管理员的数量为：\n");
    string num;
    getline(cin,num);

    printf(PLUSBLUE "注意：接下来每次只能输入一个人\n" RESET);
 
    for(int i=0;i<stoi(num) ; i++){
        printf("你想设置为管理员的人的uid为:\n");
        string manager_uid;
        getline(cin,manager_uid);

        Message msg(log_uid,OWNER_ADD_MANAGERS,group_name,manager_uid);
        socket_fd.mysend(msg.S_to_json());

        string recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
        if(recv=="quit"){
            printf("你不是群主，无法进行该操作\n");
            return;
        }

        if(recv=="have_exist"){
            printf("此人已经是管理员\n");
            return;
        }

        if(recv=="ok"){
            printf("已成功将此人添加为管理员\n");
            
        }

    }

    return;

}

void owner_del_managers(const string group_name){
    check_group_managers(group_name);

    printf("你想删除管理员的数量为：\n");
    string num;
    getline(cin,num);

    printf(PLUSBLUE "注意：接下来每次只能输入一个人\n" RESET);
 
    for(int i=0;i<stoi(num) ; i++){
        printf("你想删除管理员的人的uid为:\n");
        string manager_uid;
        getline(cin,manager_uid);

        Message msg(log_uid,OWNER_DEL_MANAGERS,group_name,manager_uid);
        socket_fd.mysend(msg.S_to_json());

        string recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
        if(recv=="quit"){
            printf("你不是群主，无法进行该操作\n");
            return;
        }

        if(recv=="have_no_exist"){
            printf("此人本来就不是管理员\n");
            return;
        }

        if(recv=="ok"){
            printf("已成功将此人从管理员中删除\n");
            
        }

    }

    return;


}

void owner_quit_group(const string group_name){
    Message msg(log_uid,OWNER_QUIT_GROUP,group_name);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    if(recv=="quit"){
        printf("你不是该群群主，无法进行该操作\n");
        return;
    }

    if(recv=="ok"){
        printf("你已成功解散该群\n");
        return;
    }

}

void all_managers_del_members(const string group_name){
    check_group_members(group_name);

    printf("你想踢出群成员的数量为：\n");
    string num;
    getline(cin,num);

    printf(PLUSBLUE "注意：接下来每次只能输入一个人\n" RESET);
 
    for(int i=0;i<stoi(num) ; i++){
        printf("你想踢出群成员的人的uid为:\n");
        string tick_uid;
        getline(cin,tick_uid);

        Message msg(log_uid,ALL_MANAGERS_DEL_MEMBERS,group_name,tick_uid);
        socket_fd.mysend(msg.S_to_json());

        string recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
        if(recv=="no_high"){
            printf("你既不是群主，也不是管理员，无法进行该操作\n");
            return;
        }

        if(recv=="no_exist"){
            printf("此人已不在该群，可能已经被群主或其他管理员踢出\n");
            return;
        }

        if(recv=="quit"){
            printf("你想提出的人为群主或其他管理员，你没有权限将他踢出\n");
            return;
        }

        if(recv=="ok"){
            printf("已成功将此人踢出该群\n");
            
        }

    }

    return;

}

void check_group_managers(const string group_name){
    Message msg(CHECK_GROUP_MANAGERS,group_name);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }

    if(recv=="no_group_name"){
        printf("不存在该群\n");
        return;
    }

    if(recv=="no"){
        printf("该群还没有管理员\n");
        return;
    }

    while(recv != "over"){
        cout<<recv<<endl;
        recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
    }

    printf("以上是该群的所有管理员\n");
}


void group_chat(const string group_name){
    Message msg(log_uid,GROUP_CHAT,group_name);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    if(recv=="no_exist"){
        printf("你不是该群的成员，无法进行聊天\n");
        return;
    }

    while(recv!="over"){
        cout<<recv<<endl;
        recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
    }

    
    printf("以上为该群的历史消息记录，可以进行新的聊天了\n");
    printf("HELP(如果你想收发文件或退出):\n");
    printf("[1]请输入 <send> 来发送文件\n");
    printf("[2]请输入 <recv> 来接受文件\n");
    printf("[3]输入 <quit> 可退出\n\n");

    while(1){
        string notice;
        getline(cin,notice);
        if(notice=="send"){
            string filepath;
            printf("请输入你要发送的文件的路径:\n");
            getline(cin, filepath);

            thread thread([uid=log_uid,friend_or_group=group_name,path=filepath](){
                sfile(uid,friend_or_group,GROUP_SEND_FILE,path);
            });
            thread.detach();
            string recv=socket_fd.client_recv();
            if (recv == "读取消息头不完整")
            {
                cout << "服务器关闭" << endl;
                exit(EXIT_SUCCESS);
            }
            if(recv=="ok"){
                continue;
            }

        }
        else if(notice=="recv"){
            string other_uid;
            printf("你要下载的文件的人的uid为\n");
            getline(cin, other_uid);

            printf("你要下载的文件名为：\n");
            string filename;
            getline(cin, filename);

            printf("请输入你想存储的文件的位置（无需以 / 结尾）：\n");
            string want_path;
            getline(cin, want_path);
            thread thread([uid=log_uid,friend_or_group=group_name,filename=filename,want_path=want_path,other_uid=other_uid](){
                gvfile(uid,friend_or_group,FRIEND_RECV_FILE,filename,want_path,other_uid);
            });
            thread.detach();
            string recv=socket_fd.client_recv();
            if (recv == "读取消息头不完整")
            {
                cout << "服务器关闭" << endl;
                exit(EXIT_SUCCESS);
            }
            if(recv=="ok"){
                continue;
            }
        }
        else if(notice=="quit"){
            Message msg(log_uid,GROUP_QUIT_CHAT,group_name);
            socket_fd.mysend(msg.S_to_json());
            string recv=socket_fd.client_recv();
            if (recv == "读取消息头不完整")
            {
                cout << "服务器关闭" << endl;
                exit(EXIT_SUCCESS);
            }
            if(recv=="ok") return;

        }

        Message msg(log_uid,GROUP_DAILY_CHAT,group_name,notice);
        socket_fd.mysend(msg.S_to_json());
        string recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
        if(recv=="ok"){
            continue;
        }
    }

}



