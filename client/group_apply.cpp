#include "client.hpp"
void group_apply_menu(){
    string opt;
    while(1){
        printf("-------------群聊申请界面-------------\n");
        printf("选项：\n[1]同意群聊申请\n[2]拒绝群聊申请\n[3]查看群聊申请\n[4]返回\n");
        printf("请输入你的选择：\n");
        getline(cin,opt);
        printf("-------------------------\n");

        
            switch (stoi(opt))
            {
            case 1:
                group_apply_agree();
                break;
            case 2:
                group_apply_refuse();
                break;
            case 3:
                check_group_apply();
                break;
            case 4:
                return;
            
            default:
                printf("请输入范围内的选择\n");
                break;
            }
        
        

    }
}



void group_apply_agree(){
    printf("你想要同意的的好友申请的数量为：\n");
    string num;
    getline(cin,num);
    
    printf("请依次输入你想同意的人的uid和群名\n");
   
    for(int i=0;i<stoi(num);i++){
        printf("你想同意入群的人的uid为:\n");
        string person_uid;
        getline(cin,person_uid);

        printf("该群的名字为：\n");
        string group_name;
        getline(cin,group_name);

        Message msg(log_uid,GROUP_APPLY_AGREE,person_uid,group_name);
        socket_fd.mysend(msg.S_to_json());

        string recv=socket_fd.client_recv();

        if(recv=="group_no_exit"){
            printf("该群聊不存在\n");
            continue;
        }

        if(recv=="have_exist"){
            printf("该人已经在该群里，可能已经被其他高权限者同意入群\n");
            continue;
        }

        if(recv=="nosend"){
            printf("对方没有给你发过群聊申请\n");
            return;
        }

        if(recv=="ok"){
            printf("已成功同意这个人进入群聊\n");
            continue;
        }

    }        

    

}

void group_apply_refuse(){
    printf("你想要拒绝的的好友申请的数量为：\n");
    string num;
    getline(cin,num);
    
    printf("请依次输入你想拒绝的人的uid和群名\n\n");
   
    for(int i=0;i<stoi(num);i++){
    

    printf("你想拒绝入群的人的uid为:\n");
    string person_uid;
    getline(cin,person_uid);

    printf("该群的名字为：");
    string group_name;
    getline(cin,group_name);

    Message msg(log_uid,GROUP_APPLY_AGREE,person_uid,group_name);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();

    if(recv=="group_no_exit"){
            printf("该群聊不存在");
            continue;
        }

    if(recv=="have_exist"){
        printf("该人已经在该群里，可能已经被其他高权限者同意入群\n");
        return;
    }

    if(recv=="ok"){
        printf("已成功拒绝这个人进入群聊");
        return;
    }

    if(recv=="nosend"){
            printf("对方没有给你发过群聊申请\n");
            return;
        }

    }        
}

void check_group_apply(){
    Message msg(log_uid,CHECK_GROUP_APPLY);
    socket_fd.mysend(msg.S_to_json());

    
    string recv = socket_fd.client_recv();

    if(recv=="no"){
        printf("你还没有未处理的群聊申请\n");
        return;
    }


    while(recv != "over"){
        cout <<recv<<endl;
        recv=socket_fd.client_recv();
    }
    

    printf("以上是所有群聊申请,可选出想同意或拒绝的人的uid 和 群聊名字,来同意或拒绝申请\n");
    return;

    
}