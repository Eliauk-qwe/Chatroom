#include "client.hpp"
void group_chat_menu(const string group_name){
    string opt;
    while(1){
        cout<<"-------------"<<group_name<<"------------"<<endl;
        printf("选项：\n[1]群聊聊天\n[2]查看群成员\n[3]退出群聊\n[4]群主添加管理员\n[5]群主删除管理员\n[6]群主解散群聊\n[7]高级权限者踢出用户\n[8]查看群管理员\n[9]返回\n");
        printf("请输入你的选择：\n");
        getline(cin,opt);
        printf("------------------------------------\n");

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

void group_chat(const string group_name){

}

void check_group_members(const string group_name){
    Message msg(CHECK_GROUP_MEMBERS,group_name);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();

    while((recv=socket_fd.client_recv()) != "over"){
        if(recv=="no_group_name"){
            printf("不存在该群\n");
            return;
        }

        cout<<recv<<endl;
    }

    printf("以上是该群的所有群成员\n");
    
}

void group_quit(const string group_name){
    Message msg(log_uid,GROUP_QUIT,group_name);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();
    if(recv=="no_group_member"){
        printf("你本来就不是该群的成员\n");
        return;
    }

    if(recv=="quit"){
        printf("你是该群群主，不能退出群聊");
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

    printf("注意：接下来每次只能输入一个人\n");
 
    for(int i=0;i<stoi(num) ; i++){
        printf("你想设置为管理员的人的uid为:\n");
        string manager_uid;
        getline(cin,manager_uid);

        Message msg(log_uid,OWNER_ADD_MANAGERS,group_name,manager_uid);
        socket_fd.mysend(msg.S_to_json());

        string recv=socket_fd.client_recv();
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

    printf("注意：接下来每次只能输入一个人\n");
 
    for(int i=0;i<stoi(num) ; i++){
        printf("你想删除管理员的人的uid为:\n");
        string manager_uid;
        getline(cin,manager_uid);

        Message msg(log_uid,OWNER_ADD_MANAGERS,group_name,manager_uid);
        socket_fd.mysend(msg.S_to_json());

        string recv=socket_fd.client_recv();
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

    printf("注意：接下来每次只能输入一个人\n");
 
    for(int i=0;i<stoi(num) ; i++){
        printf("你想踢出群成员的人的uid为:\n");
        string tick_uid;
        getline(cin,tick_uid);

        Message msg(log_uid,ALL_MANAGERS_DEL_MEMBERS,group_name,tick_uid);
        socket_fd.mysend(msg.S_to_json());

        string recv=socket_fd.client_recv();
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

    while((recv=socket_fd.client_recv()) != "over"){
        if(recv=="no_group_name"){
            printf("不存在该群\n");
            return;
        }

        cout<<recv<<endl;
    }

    printf("以上是该群的所有管理员\n");
}