#include "client.hpp"
void new_friend()
{
    string opt;
    while (1)
    {
        printf("=============新的朋友界面=============\n");
        printf("选项：\n[1]同意好友申请\n[2]拒绝好友申请\n[3]查看好友申请\n[4]返回\n");
        printf("请输入你的选择：\n");
        getline(cin, opt);
        printf("====================================\n");

        switch (stoi(opt))
        {
        case 1:
            friend_apply_agree();
            break;
        case 2:
            friend_apply_refuse();
            break;
        case 3:
            check_friend_apply();
            break;
        case 4:
            return;

        default:
            printf("请输入范围内的选择\n");
            break;
        }
    }
}

void friend_apply_agree(){
    int res=check_friend_apply();
    if(res<0){
        return;
    }
    printf("\n你想要同意的的好友申请的数量为:\n");
    string num;
    getline(cin,num);

    printf(PLUSBLUE "请依次输入你想添加的好友的uid(不能为空)\n\n" RESET);
    for (int i = 0; i < stoi(num); i++)
    {

        string friend_agree_uid;
        printf("你想同意的收到的好友申请的uid:\n");
        getline(cin, friend_agree_uid);

        Message msg(log_uid, FRIEND_APPLY_AGREE, friend_agree_uid);
        socket_fd.mysend(msg.S_to_json());

        string recv = socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }

        if (recv == "ok")
        {
            printf("你们已成功加为好友\n");
            continue;
        }
    }
}







void friend_apply_refuse(){
    int res=check_friend_apply();
    if(res<0){
        return;
    }
    printf("\n你想要拒绝的的好友申请的数量为:\n");
    string num;
    getline(cin, num);

    printf(PLUSBLUE "请依次输入你想拒绝的好友的uid(不能为空)\n\n" RESET);

    for (int i = 0; i < stoi(num); i++)
    {
        string friend_refuse_uid;
        printf("你想拒绝的收到的好友申请的uid:\n");
        getline(cin, friend_refuse_uid);

        Message msg(log_uid, FRIEND_APPLY_REFUSE, friend_refuse_uid);
        socket_fd.mysend(msg.S_to_json());

        string recv = socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }

        if (recv == "OK")
        {
            printf("你们已成功拒绝加为好友\n");
            continue;
        }
    }
}


/*void check_friend_apply(){
    Message msg(log_uid,CHECK_FRIEND_APPLY);
    socket_fd.mysend(msg.S_to_json());

    string recv;
    

    while ((recv=socket_fd.client_recv()) != "over")
    {
        getchar();
        if(recv=="no"){
            getchar();
            cout<<"你还没有好友申请"<<endl;
            return;
        }
        cout << recv <<endl;
    }
    

    printf("以上是所有好友申请,可选出想添加好友的uid,来同意或拒绝申请\n");
    return;



}*/

int check_friend_apply() {
    printf("\n");

    Message msg(log_uid, CHECK_FRIEND_APPLY);
    socket_fd.mysend(msg.S_to_json());

    string recv;
    // 先接收一次数据，判断是否有申请
    recv = socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    
    if (recv == "no") {
        cout << "你还没有好友申请" << endl;
        return -1;
    }
    
    // 如果有申请，循环接收直到"over"
    while (recv != "over") {
        cout << recv << endl;
        recv = socket_fd.client_recv();// 继续接收下一条
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        } 
    }

    printf("以上是所有好友申请,可选出想添加好友的uid,来同意或拒绝申请\n");
    return 0;
}