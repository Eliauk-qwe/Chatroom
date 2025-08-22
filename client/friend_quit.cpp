#include "client.h"
void friend_quit_menu()
{
    string opt;
    while (1)
    {
        printf("======================屏蔽好友界面==================\n");
        printf("选项：\n[1]屏蔽好友\n[2]屏蔽好友列表\n[3]恢复好友\n[4]返回\n");
        printf("请输入你的选择：\n");
        getline(cin, opt);
        printf("==================================================\n");
        if (isNotNumber(opt))
        {
            std::cout << "不是数字" << std::endl;
            continue;
        }

        switch (stoi(opt))
        {
        case 1:
            friend_quit();
            break;
        case 2:
            friend_quit_list();
            break;
        case 3:
            friend_back();
            break;
        case 4:
            return;

        default:
            printf("请输入正确选项\n");
            break;
        }
    }
}

void friend_quit()
{
    int res = friend_list();
    if (res < 0)
    {
        return;
    }
    /*string num;
    printf("\n你想屏蔽好友的数量为:\n");
    getline(cin, num);

    printf(PLUSBLUE "注意：接下来每次只能输入一个人(不能为空)\n" RESET);


    for (int i = 0; i < stoi(num); i++)
    {*/

    string friend_quit_uid;
    cout << "你想屏蔽的好友uid为:" << endl;
    getline(cin, friend_quit_uid);

    Message msg(log_uid, FRIEND_QUIT, friend_quit_uid);
    socket_fd.mysend(msg.S_to_json());

    string recv = socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    if (recv == "friend_no_exist")
    {
        printf("你未添加该好友\n");
        return;
    }
    else if (recv == "ok")
    {
        printf("屏蔽成功\n");
        return;
    }
    else if (recv == "friend_have_exist")
    {
        printf("你之前已屏蔽过该好友\n");
        return;
    }

    return;
}

int friend_quit_list()
{
    printf("\n");

    Message msg(log_uid, FRIEND_QUIT_LIST);
    socket_fd.mysend(msg.S_to_json());

    string recv;

    recv = socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }

    if (recv == "no_have_quit_list")
    {
        printf("你没有屏蔽的好友\n");
        return -1;
    }

    while (recv != "over")
    {
        cout << recv << endl;
        recv = socket_fd.client_recv();
    }

    printf("以上是你的好友屏蔽列表\n");
    return 0;
}

void friend_back()
{
    int res = friend_quit_list();
    if (res < 0)
    {
        return;
    }
    /*string num;
    printf("\n你想取消屏蔽好友的数量为;\n");
    getline(cin, num);

    printf(PLUSBLUE "注意：接下来每次只能输入一个人(不能为空)\n" RESET);


    for (int i = 0; i < stoi(num); i++)
    {*/
    string friend_back_uid;
    printf("请输入你想取消屏蔽的好友的uid:\n");
    getline(cin, friend_back_uid);
    Message msg(log_uid, FRIEND_BACK, friend_back_uid);
    socket_fd.mysend(msg.S_to_json());

    string recv = socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    if (recv == "no_quit_freind")
    {
        printf("你没有屏蔽过该好友\n");
        return;
    }
    else if (recv == "no_add")
    {
        printf("你没有添加过该好友\n");
        return;
    }
    else if (recv == "ok")
    {
        printf("你已成功恢复好友\n");
        return;
    }
}