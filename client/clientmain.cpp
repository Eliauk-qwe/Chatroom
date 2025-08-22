#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include "client.h"
#include <csignal>

using namespace std;
StickyPacket socket_fd("start");
sockaddr_in client_addr;
string log_uid = "0";

int main(int argc, char *argv[])
{
    std::srand(std::time(0));

    if (argc != 3)
    {
        cerr << "Usage : " << argv[0] << " <IP> <PORT>" << endl;
        exit(EXIT_FAILURE);
    }

    int fd = socket_fd.getfd();

    // 设置发送缓冲区大小（系统级）
    int send_buf_size = 2 * 1024 * 1024; // 1MB
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof(send_buf_size));

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    uint16_t port = atoi(argv[2]);
    client_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, argv[1], (sockaddr *)&client_addr.sin_addr) < 0)
    {
        perror("inet_pton failed!\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(client_addr);
    if (connect(fd, (sockaddr *)&client_addr, len) < 0)
    {
        perror("connect failed!\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int res;
    // 命令循环
    while (1)
    {
        cout << "===============欢迎来到聊天室================" << endl;
        printf("选项:\n[1]注册新账户\n[2]登录账户\n[3]找回密码\n[4]退出\n输入选择: \n");
        string opt;
        getline(cin, opt);
        printf("=========================================\n");

        if (isNotNumber(opt))
        {
            std::cout << "不是数字" << std::endl;
            continue;
        }

        switch (stoi(opt))
        {
        case 1:
        {
            sign_up();
            break;
        }
        case 2:
        {
            res = log_in();
            if (res == 1)
                user_menu();
            break;
        }
        case 3:
        {
            pass_find();
            break;
        }
        case 4:
        {
            exit(0);
        }
        default:
            printf("请输入正确选项\n");
            break;
        }
    }
}