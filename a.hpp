#ifndef STICKY_PACKET_HPP
#define STICKY_PACKET_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <cstring>
#include <cstdint>
#include <netinet/in.h>

class StickyPacket {
public:
    //StickyPacket() = default;
    //StickyPacket(int fd) : sockfd(fd) {}

    int getfd() {
        return sockfd;
    }

    int get_notice_fd(){
        return notice_fd;
    }

    StickyPacket(){
        sockfd=socket(AF_INET,SOCK_STREAM,0);
    }

    StickyPacket(int fd){
        this->sockfd=fd;
    }

    StickyPacket(std:: string msg){
        if(msg=="receive"){
            sockfd=socket(AF_INET,SOCK_STREAM,0);
            notice_fd=socket(AF_INET,SOCK_STREAM,0);
        }
    }

    ~StickyPacket()  {
        //1cout<<"StickyPacket 析构函数调用"<<endl;

    }

    

    // 从 socket 读取完整数据包
    // 返回值：
    //   >0  成功读取数据（client_cmd 保存数据）
    //   =0  对方关闭连接
    //   <0  出错
    int notice_recv(std::string &client_cmd)
    {
        if (sockfd < 0)
        {
            // sockfd 未初始化，返回错误
            return -1;
        }
        char buf[4096];
        int total_bytes = 0;

        while (true)
        {
            ssize_t n = recv(sockfd, buf, sizeof(buf), 0);
            if (n > 0)
            {
                total_bytes += n;
                buffer[sockfd].insert(buffer[sockfd].end(), buf, buf + n);
            }
            else if (n == 0)
            {
                // 对端关闭连接
                return 0;
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    break; // 数据读完了
                }
                perror("recv");
                return -1;
            }
        }

        while (true)
        {
            if (buffer[sockfd].size() < sizeof(uint32_t))
            {
                break; // 不够读取长度字段
            }
            uint32_t msg_len;
            memcpy(&msg_len, buffer[sockfd].data(), sizeof(uint32_t));
            msg_len = ntohl(msg_len);

            if (buffer[sockfd].size() < sizeof(uint32_t) + msg_len)
            {
                break; // 不够完整消息
            }

            client_cmd.assign(buffer[sockfd].begin() + sizeof(uint32_t),
                              buffer[sockfd].begin() + sizeof(uint32_t) + msg_len);

            buffer[sockfd].erase(buffer[sockfd].begin(),
                                 buffer[sockfd].begin() + sizeof(uint32_t) + msg_len);

            return total_bytes;
        }

        return total_bytes > 0 ? total_bytes : -1;
    }

    // 向 socket 发送数据（自动添加包长度）
    bool mysend(const std::string &data) {
        uint32_t len = htonl(static_cast<uint32_t>(data.size()));
        std::vector<char> send_buf(sizeof(uint32_t) + data.size());
        memcpy(send_buf.data(), &len, sizeof(uint32_t));
        memcpy(send_buf.data() + sizeof(uint32_t), data.data(), data.size());

        ssize_t sent = send(sockfd, send_buf.data(), send_buf.size(), 0);
        return sent == static_cast<ssize_t>(send_buf.size());
    }

private:
    int sockfd{-1};
    int notice_fd=-1;

    // 每个 fd 对应一个缓冲区，解决多连接粘包问题
    static std::unordered_map<int, std::vector<char>> buffer;
};

// 静态成员初始化
//std::unordered_map<int, std::vector<char>> StickyPacket::buffer;

#endif // STICKY_PACKET_HPP
