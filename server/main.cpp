#include "server.hpp"
#include <csignal>
#include <thread>
#include <mutex>
#include <fcntl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <atomic>

using namespace std;

MessageTrans trans;
Redis redis;
unordered_set<string> online_users;
// 静态成员初始化
std::unordered_map<int, std::vector<char>> StickyPacket::buffer;

// ===== 活跃时间表 & fd→用户uid映射 & 互斥锁 =====
std::unordered_map<int, std::chrono::steady_clock::time_point> heart_time;
std::unordered_map<int, std::string> fd_to_user;
std::mutex heart_mutex;       // 保护 heart_time
std::mutex fd_mutex;          // 保护 fd_to_user
std::mutex online_mutex;      // 保护 online_users

std::chrono::seconds maxtime = std::chrono::seconds(60);

int epoll_fd = -1; // 全局 epoll 文件描述符
std::atomic<bool> running{true};

void setnoblock(int fd) {
    int flag = fcntl(fd, F_GETFL);
    if (flag < 0) {
        cerr << "fcntl F_GETFL failed: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) < 0) {
        cerr << "fcntl F_SETFL failed: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
}

// 心跳线程：定期扫描 heart_time，超时则从 epoll 删除并 close
/*void heart() {
    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        auto now = std::chrono::steady_clock::now();

        vector<int> time_out_clients;
        {
            std::lock_guard<std::mutex> lock(heart_mutex);
            for (const auto &p : heart_time) {
                if (now - p.second > maxtime) {
                    time_out_clients.push_back(p.first);
                }
            }
        }

        for (int fd : time_out_clients) {
            // 统一锁定顺序：先 heart_mutex 再 fd_mutex 再 online_mutex
            // 使用 scoped_lock 保证不会死锁
            {
                std::scoped_lock locks(heart_mutex, fd_mutex, online_mutex);

                // remove from epoll first
                if (epoll_fd >= 0) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                }

                // remove from heart_time
                heart_time.erase(fd);

                // remove fd->user 映射并从 online_users 中删除对应 uid（如果有）
                auto it = fd_to_user.find(fd);
                if (it != fd_to_user.end()) {
                    const std::string uid = it->second;
                    fd_to_user.erase(it);
                    if (!uid.empty() && online_users.find(uid) != online_users.end()) {
                        online_users.erase(uid);
                    }
                    // 如果你确实需要维护 redis 映射，可以在此更新 redis（但避免高频 redis 操作）
                    if (redis.Exists("客户端fd与对应uid表")) {
                        redis.Hdel("客户端fd与对应uid表", to_string(fd));
                    }
                }
            } // unlock

            // 最后 close（在解锁后做 close 避免长时间持锁）
            close(fd);
            cout << "客户端 " << fd << " 心跳超时已断开连接" << endl;
        }
    }
}*/

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);

    // 初始化 redis 计数器（如果不存在）
    if (!redis.Exists("user_uid_counter")) {
        redis.set("user_uid_counter", "0");
    }
    if (!redis.Exists("group_uid_counter")) {
        redis.set("group_uid_counter", "0");
    }

    if (argc != 3) {
        cerr << "Usage : " << argv[0] << " <IP> <PORT>" << endl;
        exit(EXIT_FAILURE);
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        cerr << "Invalid server IP" << endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    uint32_t port = atoi(argv[2]);
    server_addr.sin_port = htons(port);

    // 允许端口复用
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 增大接收缓冲区（可选）
    int buf_size = 2 * 1024 * 1024; // 2MB
    setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));

    if (bind(server_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, LISTEN_NUM) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // epoll 创建
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    setnoblock(server_fd);
    struct epoll_event ev;
    ev.data.fd = server_fd;
    ev.events = EPOLLIN | EPOLLET; // 边沿触发
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("epoll_ctl ADD server_fd");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 线程池（示例）
    ThreadPool pool(20);

    // 启动心跳线程
   /* std::thread heart_thread(heart);
    heart_thread.detach();*/

    struct epoll_event events[MAX_EVENTS];

    while (true) {
        int count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (count < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        // cout 可以在调试时打开
        // cout << "Epoll returned " << count << " events" << endl;

        for (int i = 0; i < count; ++i) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                // ET 模式下 accept 需要循环直到 EAGAIN
                while (true) {
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int new_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
                    if (new_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // 没有更多连接
                            break;
                        } else {
                            perror("accept");
                            break;
                        }
                    }

                    printf("New connection: socket %d\n", new_fd);

                    setnoblock(new_fd);
                    struct epoll_event client_ev;
                    client_ev.data.fd = new_fd;
                    client_ev.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &client_ev) < 0) {
                        perror("epoll_ctl ADD new_fd");
                        close(new_fd);
                        continue;
                    }

                    // 记录初始心跳时间（注意锁保护）
                    {
                        std::lock_guard<std::mutex> lock(heart_mutex);
                        heart_time[new_fd] = std::chrono::steady_clock::now();
                    }

                    // 可选：把 new_fd 与 redis 映射初始化（根据你的逻辑）
                }
            } else if (events[i].events & EPOLLIN) {
                // 更新活跃时间（尽快做，最小持锁时间）
                {
                    std::lock_guard<std::mutex> lock(heart_mutex);
                    heart_time[fd] = std::chrono::steady_clock::now();
                }

                // 读取数据（你的 StickyPacket::notice_recv 应该处理 ET 半包）
                StickyPacket sp(fd);
                string client_cmd;
                int ret = sp.notice_recv(client_cmd);

                if (ret <= 0) {
                    if (ret == 0) {
                        cout << "Client closed connection. fd: " << fd << endl;
                    } else {
                        perror("recv error");
                    }

                    // 安全清理：按固定锁顺序 heart -> fd -> online
                    {
                        std::scoped_lock locks(heart_mutex, fd_mutex, online_mutex);
                        heart_time.erase(fd);
                        auto it = fd_to_user.find(fd);
                        if (it != fd_to_user.end()) {
                            const string uid = it->second;
                            fd_to_user.erase(it);
                            if (!uid.empty() && online_users.find(uid) != online_users.end()) {
                                cout<<"111111"<<endl;
                                online_users.erase(uid);
                            }
                        }
                    }

                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                    continue;
                }

                // 正常收到命令，打印并处理
                cout << "收到 fd=" << fd << " : " << client_cmd << endl;

                if (client_cmd.find("\"flag\":\"2\"") != std::string::npos) {
                    // 解析并记录 fd -> uid（受 fd_mutex 保护）
                    Message msg;
                    msg.Json_to_s(client_cmd);
                    {
                        std::lock_guard<std::mutex> lock(fd_mutex);
                        fd_to_user[fd] = msg.uid;
                    }
                    // 同步更新在线用户集合（可选）
                    if (!msg.uid.empty()) {
                        std::lock_guard<std::mutex> lock2(online_mutex);
                        online_users.insert(msg.uid);
                    }
                }

                // 将任务加入线程池（拷贝必要数据）
                // 注意：StickyPacket sp 按值捕获，确保内部数据在子线程可用
                Task task([sp, client_cmd]() mutable {
                    trans.translation(sp, client_cmd);
                });
                pool.addTask(task);
            } else {
                // 处理其它事件（错误、挂起等）
                if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                    // 发生异常，安全清理
                    cout << "EPOLL error on fd " << fd << endl;
                    {
                        std::scoped_lock locks(heart_mutex, fd_mutex, online_mutex);
                        heart_time.erase(fd);
                        auto it = fd_to_user.find(fd);
                        if (it != fd_to_user.end()) {
                            const string uid = it->second;
                            fd_to_user.erase(it);
                            if (!uid.empty() && online_users.find(uid) != online_users.end()) {
                                online_users.erase(uid);
                            }
                        }
                    }
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                }
            }
        }
    }

    // 程序退出前清理（如果需要）
    running.store(false);
    close(epoll_fd);
    close(server_fd);
    return 0;
}
