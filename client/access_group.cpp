#include "client.h"
void group_owner_menu(const string groupID,const string group_name){

    string opt;
    while(1){
        cout<<"================"<<group_name<<"=============="<<endl;
        printf("选项：\n[1]群聊聊天\n[2]查看群成员\n[3]退出群聊\n[4]群主添加管理员\n[5]群主删除管理员\n[6]群主解散群聊\n[7]群主踢出用户\n[8]查看群管理员\n[9]邀请好友加群[10]返回\n");
        printf("请输入你的选择：\n");
        getline(cin,opt);
        printf("=============================================\n");
        if (isNotNumber(opt))
        {
            std::cout << "不是数字" << std::endl;
            continue;
        }

        switch (stoi(opt))
        {
        case 1:
            group_chat(groupID);
            break;
        case 2:
            check_group_members(groupID);
            break;
        case 3:
            group_quit(groupID);
            break;
        case 4:
            owner_add_managers(groupID);
            break;
        case 5:
            owner_del_managers(groupID);
            break;
        case 6:
            owner_quit_group(groupID);
            break;
        case 7:
            all_managers_del_members(groupID);
            break;
        case 8:
            check_group_managers(groupID);
            break;
        case 9:
            invite_friend_to_group(groupID);
            return;
        case 10:
            return;
       default:
            printf("请输入正确选项\n");
            break;
        }
    }
}



void group_common_menu(const string groupID,const string group_name){

    string opt;
    while(1){
        cout<<"================"<<group_name<<"=============="<<endl;
        printf("选项：\n[1]群聊聊天\n[2]查看群成员\n[3]退出群聊\n[4]返回\n");
        printf("请输入你的选择：\n");
        getline(cin,opt);
        printf("=============================================\n");
        if (isNotNumber(opt))
        {
            std::cout << "不是数字" << std::endl;
            continue;
        }
        switch (stoi(opt))
        {
        case 1:
            group_chat(groupID);
            break;
        case 2:
            check_group_members(groupID);
            break;
        case 3:
            group_quit(groupID);
            break;
        case 4:
            return;
        default:
            printf("请输入正确选项\n");
            break;
        }
    }
}



void group_manager_menu(const string groupID,const string group_name){
    string opt;
    while(1){
        cout<<"================"<<group_name<<"=============="<<endl;
        printf("选项：\n[1]群聊聊天\n[2]查看群成员\n[3]退出群聊\n[4]管理员踢出用户\n[5]查看群管理员\n[6]返回\n");
        printf("请输入你的选择：\n");
        getline(cin,opt);
        printf("=============================================\n");
        if (isNotNumber(opt))
        {
            std::cout << "不是数字" << std::endl;
            continue;
        }

        switch (stoi(opt))
        {
        case 1:
            group_chat(groupID);
            break;
        case 2:
            check_group_members(groupID);
            break;
        case 3:
            group_quit(groupID);
            break;
        case 4:
            all_managers_del_members(groupID);
            break;
        case 5:
            check_group_managers(groupID);
            break;
        case 6:
            return;
        default:
            printf("请输入正确选项\n");
            break;
        }
    }
}




int check_group_members(const string groupID){
    Message msg(log_uid,CHECK_GROUP_MEMBERS,groupID);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    if(recv=="no_group_name"){
        printf("不存在该群\n");
        return 0;
    }
    if(recv=="no_group_member"){
        printf("你不是该群的成员,无法查看群成员\n");
        return 0;
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

    return 0;
    
}

void group_quit(const string groupID){
    Message msg(log_uid,GROUP_QUIT,groupID);
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

void owner_add_managers(const string groupID){
    int res=check_group_members(groupID);
    if(res<0){
        return;
    }

    /*printf("你想设置为管理员的数量为：\n");
    string num;
    getline(cin,num);

    printf(PLUSBLUE "注意：接下来每次只能输入一个人\n" RESET);
 
    for(int i=0;i<stoi(num) ; i++){*/
        printf("你想设置为管理员的人的uid为:\n");
        string manager_uid;
        getline(cin,manager_uid);

        Message msg(log_uid,OWNER_ADD_MANAGERS,groupID,manager_uid);
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

    

    return;

}

void owner_del_managers(const string groupID){
    int res=check_group_managers(groupID);
    if(res<0){
        return;
    }

    /*printf("你想删除管理员的数量为：\n");
    string num;
    getline(cin,num);

    printf(PLUSBLUE "注意：接下来每次只能输入一个人\n" RESET);
 
    for(int i=0;i<stoi(num) ; i++){*/
        printf("你想删除管理员的人的uid为:\n");
        string manager_uid;
        getline(cin,manager_uid);

        Message msg(log_uid,OWNER_DEL_MANAGERS,groupID,manager_uid);
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

    

    return;


}

void owner_quit_group(const string groupID){
    Message msg(log_uid,OWNER_QUIT_GROUP,groupID);
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

void all_managers_del_members(const string groupID){
    check_group_members(groupID);

    /*printf("你想踢出群成员的数量为：\n");
    string num;
    getline(cin,num);

    printf(PLUSBLUE "注意：接下来每次只能输入一个人\n" RESET);
 
    for(int i=0;i<stoi(num) ; i++){*/
        printf("你想踢出群成员的人的uid为:\n");
        string tick_uid;
        getline(cin,tick_uid);

        Message msg(log_uid,ALL_MANAGERS_DEL_MEMBERS,groupID,tick_uid);
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

        if(recv=="my"){
            printf("不能自己踢出自己\n");
            return;
        }

        if(recv=="ok"){
            printf("已成功将此人踢出该群\n");
            
        }

    

    return;

}

int check_group_managers(const string groupID){
    Message msg(CHECK_GROUP_MANAGERS,groupID);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }

    if(recv=="no_group_name"){
        printf("不存在该群\n");
        return 0;
    }

    if(recv=="0"){
        printf("该群还没有管理员\n");
        return -1;
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

    return 0;
}


void group_chat(const string groupID){
    Message msg(log_uid,GROUP_CHAT,groupID);
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

    printf("现在可以开始新的聊天了:\n");
    printf(PLUSBLUE "HELP(如果你想收发文件或退出): \n" RESET);
    printf(PLUSBLUE "[1]请输入 :send 来发送文件\n" RESET);
    printf(PLUSBLUE "[2]请输入 :recv 来接受文件\n" RESET);
    printf(PLUSBLUE "[3]输入 :quit 可退出\n\n" RESET);

    while(1){
        string notice;
        getline(cin,notice);
        

        if(notice==":send"){
            string filepath;
            printf("请输入你要发送的文件的路径:\n");
            getline(cin, filepath);

            thread thread([uid=log_uid,friend_or_group=groupID,path=filepath](){
                sfile(uid,friend_or_group,GROUP_SEND_FILE,path);
            });
            thread.detach();
            /*string recv=socket_fd.client_recv();
            if (recv == "读取消息头不完整")
            {
                cout << "服务器关闭" << endl;
                exit(EXIT_SUCCESS);
            }
            if(recv=="ok"){
                continue;
            }*/
           continue;

        }
        else if(notice==":recv"){
            string other_uid;
            printf("你要下载的文件的人的uid为\n");
            getline(cin, other_uid);

            printf("你要下载的文件名为：\n");
            string filename;
            getline(cin, filename);

            printf("请输入你想存储的文件的位置（无需以 / 结尾）：\n");
            string want_path;
            getline(cin, want_path);
            thread thread([uid=log_uid,friend_or_group=groupID,filename=filename,want_path=want_path,other_uid=other_uid](){
                gvfile(uid,friend_or_group,GROUP_RECV_FILE,filename,want_path,other_uid);
            });
            thread.detach();
            continue;
            /*string recv=socket_fd.client_recv();
            if (recv == "读取消息头不完整")
            {
                cout << "服务器关闭" << endl;
                exit(EXIT_SUCCESS);
            }
            if(recv=="ok"){
                continue;
            }*/
        }
        else if(notice==":quit"){
            Message msg(log_uid,GROUP_QUIT_CHAT,groupID);
            socket_fd.mysend(msg.S_to_json());
            string recv=socket_fd.client_recv();
            if (recv == "读取消息头不完整")
            {
                cout << "服务器关闭" << endl;
                exit(EXIT_SUCCESS);
            }
            if(recv=="ok") return;

        }

        Message msg(log_uid,GROUP_DAILY_CHAT,groupID,notice);
        socket_fd.mysend(msg.S_to_json());
        string recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }

        if(recv=="del"){
            cout<<recv<<endl;
            printf(RED "        你已不是该群成员\n" RESET);
            continue;
        }
        if(recv=="ok"){
            continue;
        }
    }

}

void invite_friend_to_group(const string groupID){
   int res= friend_list();
   if(res<0){
    return;
   }

   printf("你想邀请的好友的uid为:\n");
   string friend_uid;
   getline(cin,friend_uid);

   Message msg(log_uid,INVITE_FRIEND_TO_GROUP,groupID,friend_uid);
   socket_fd.mysend(msg.S_to_json());
   string recv=socket_fd.client_recv();


   if(recv=="no_friend"){
      printf("该用户不是你的好友\n");
      return;
   }

   if(recv=="no_sign_up"){
     printf("该用户未注册\n");
     return;
   }

   if(recv=="你已被对方删除，无法邀请"){
    cout<<recv<<endl;
    return;
   }

   if(recv=="ok"){
      printf("已成功发送邀请\n");
   }



}

