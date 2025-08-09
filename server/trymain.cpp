#include "server.hpp"
#include <csignal>
#include <vector>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <unistd.h> 

// 全局变量（需要加锁保护）
std::mutex online_users_mutex;
std::unordered_set<string> online_users;
Redis redis; // Redis连接对象

// 设置文件描述符为非阻塞模式
void setnoblock(int fd) {
    int flag = fcntl(fd, F_GETFL);
    if (flag < 0) {
        cerr << "fcntl" << endl;
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) < 0) {
        cerr << "fcntl" << endl;
        exit(EXIT_FAILURE);
    }
}

// 从Reactor类
class SubReactor {
private:
    int epoll_fd;
    int notify_recv_fd;  // 管道接收端
    int notify_send_fd;  // 管道发送端
    std::mutex queue_mutex;
    std::queue<int> new_connections;
    std::unordered_map<int, std::chrono::steady_clock::time_point> heart_time;
    //std::unordered_map<int, std::string> fd_to_uid;  // 存储fd到uid的映射
    Redis redis;  // 每个SubReactor有自己的Redis连接
    ThreadPool* pool;
    const std::chrono::seconds maxtime = std::chrono::seconds(60);
    MessageTrans trans;  // 消息处理器
    std::mutex heart_mutex;


public:
    SubReactor(ThreadPool* pool) : pool(pool) {
        // 创建epoll实例
        epoll_fd = epoll_create1(0);
        if (epoll_fd < 0) {
            perror("epoll_create1 in SubReactor");
            exit(EXIT_FAILURE);
        }

        // 创建管道用于主从通信
        int fds[2];
        if (pipe(fds) ){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        notify_recv_fd = fds[0];
        notify_send_fd = fds[1];
        setnoblock(notify_recv_fd);

        // 监听管道读端
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = notify_recv_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, notify_recv_fd, &ev) < 0) {
            perror("epoll_ctl add notify_fd");
            exit(EXIT_FAILURE);
        }

        
    }

    // 添加新连接
    void add_connection(int fd) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            new_connections.push(fd);
        }
        char buf[1] = {0};
        write(notify_send_fd, buf, 1); // 通知有新的连接
    }

    // 运行从Reactor
    void run() {
        struct epoll_event events[MAX_EVENTS];
        while (true) {
            int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000); // 1秒超时
            if (n < 0) {
                perror("epoll_wait in SubReactor");
                continue;
            }

            for (int i = 0; i < n; i++) {
                int fd = events[i].data.fd;
                if (fd == notify_recv_fd) {
                    // 处理新连接通知
                    char buf[256];
                    read(notify_recv_fd, buf, sizeof(buf)); // 清空管道

                    std::queue<int> temp_queue;
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex);
                        std::swap(temp_queue, new_connections);
                    }

                    while (!temp_queue.empty()) {
                        int new_fd = temp_queue.front();
                        temp_queue.pop();
                        setnoblock(new_fd);

                        struct epoll_event ev;
                        ev.events = EPOLLIN | EPOLLET;
                        ev.data.fd = new_fd;
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &ev) < 0) {
                            perror("epoll_ctl add new_fd");
                            close(new_fd);
                            continue;
                        }
                       //heart_time[new_fd] = std::chrono::steady_clock::now();
                        cout << "SubReactor添加新连接: " << new_fd << endl;
                    }
                } else if (events[i].events & EPOLLIN) {
                    //cout<<"fd:"<<fd<<endl;
                    // 处理数据读取
                    StickyPacket sp_fd(fd);
                    string client_cmd;
                    cout<<client_cmd<<endl;
                    
                    int recv_ret = sp_fd.notice_recv(client_cmd);
                    if (recv_ret <= 0) {
                        if (recv_ret == 0) {
                            cout << "客户端关闭连接: " << fd << endl;
                        } else {
                            //perror("接收数据错误");
                        }
                        
                        // 清理资源
                        close(fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                        if (redis.Exists("客户端fd与对应uid表"))
                        {
                            string uid = redis.Hget("客户端fd与对应uid表", to_string(fd));
                            redis.Hdel("客户端fd与对应uid表", to_string(fd));
                            if (online_users.find(uid) != online_users.end())
                            {
                                online_users.erase(uid);
                            }
                        }

                        heart_time.erase(fd);
                        cout << "客户端" << fd << "已断开连接" << endl;
                        continue;
                    }

                    // 处理消息
                   /* Message msg;
                    msg.Json_to_s(client_cmd);
                    
                    if (msg.flag == NOTICE) {
                        redis.hset(msg.uid, "消息fd", to_string(fd));
                    } else if (msg.flag == FRIEND_SEND_FILE || msg.flag == FRIEND_RECV_FILE ||
                               msg.flag == GROUP_SEND_FILE || msg.flag == GROUP_RECV_FILE) {
                        // 文件传输特殊处理
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                        thread filethread([sp_fd, client_cmd, fd, this]() {
                            trans.translation(sp_fd, client_cmd);
                            struct epoll_event aev;
                            aev.data.fd = fd;
                            aev.events = EPOLLIN | EPOLLET;
                            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &aev);
                        });
                        filethread.detach();
                    } else if (msg.flag == HEART) {
                        //cout<<"fd888888:"<<fd<<endl;
                        
                        // 更新心跳时间
                        heart_time[fd] = std::chrono::steady_clock::now();
                        redis.hset("客户端fd与对应uid表",to_string(fd),msg.uid);
                        //fd_to_uid[fd] = msg.uid;
                        //cout << "收到客户端" << fd << "的心跳包, uid: " << msg.uid << endl;
                        cout<<RED "心跳" RESET<<endl;
                    } else {
                        // 普通消息交给线程池处理
                        StickyPacket socket(fd);
                        Task task([socket, client_cmd,this]() {
                            trans.translation(socket, client_cmd);
                        });
                        pool->addTask(task);
                    }*/
                }
            }
            
            // 心跳检测
            auto now = std::chrono::steady_clock::now();
            std::vector<int> time_out_fds;
            for (const auto& [fd, last_time] : heart_time) {
                if (now - last_time > maxtime) {
                    time_out_fds.push_back(fd);
                }
            }
            
             for (int fd : time_out_fds)
            {

                {
                    std::lock_guard<std::mutex> lock(heart_mutex);
                    close(fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    if (redis.Exists("客户端fd与对应uid表"))
                    {
                        string uid = redis.Hget("客户端fd与对应uid表", to_string(fd));
                        redis.Hdel("客户端fd与对应uid表", to_string(fd));
                        if(online_users.find(uid)!=online_users.end()){
                            online_users.erase(uid);
                        }
                    }
                    heart_time.erase(fd);

                }
               // cout<<"0"<<endl;

                cout << "客户端" << fd << "已断开连接" << endl;
               
            }
        }
    }
};

// 主Reactor类
class MainReactor {
private:
    int epoll_fd;
    int server_fd;
    std::vector<SubReactor*> sub_reactors;
    int next_reactor_index;

public:
    MainReactor(int server_fd, ThreadPool* pool, int sub_reactor_count = 4) 
        : server_fd(server_fd), next_reactor_index(0) {
        // 创建epoll实例
        epoll_fd = epoll_create1(0);
        if (epoll_fd < 0) {
            perror("epoll_create1 in MainReactor");
            exit(EXIT_FAILURE);
        }

        // 监听服务器socket
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = server_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
            perror("epoll_ctl add server_fd");
            exit(EXIT_FAILURE);
        }

        // 创建从Reactor
        for (int i = 0; i < sub_reactor_count; ++i) {
            sub_reactors.push_back(new SubReactor(pool));
        }
    }

    // 获取从Reactor列表
    std::vector<SubReactor*>& get_sub_reactors() {
        return sub_reactors;
    }

    // 运行主Reactor
    void run() {
        struct epoll_event events[MAX_EVENTS];
        while (true) {
            int count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            if (count < 0) {
                perror("epoll_wait in MainReactor");
                continue;
            }

            for (int i = 0; i < count; i++) {
                if (events[i].data.fd == server_fd) {
                    // 接受新连接
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int new_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
                    if (new_fd < 0) {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            perror("accept");
                        }
                        continue;
                    }
                    setnoblock(new_fd);
                    cout << "主Reactor接受新连接: " << new_fd << endl;

                    // 轮询分配给从Reactor
                    sub_reactors[next_reactor_index]->add_connection(new_fd);
                    next_reactor_index = (next_reactor_index + 1) % sub_reactors.size();
                }
            }
        }
    }
};



int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);

    // 在main函数中初始化USER_UID计数器
    if (!redis.Exists("user_uid_counter")) {
        redis.set("user_uid_counter", "0"); // 从1000开始计数
    }

    // 在main函数中初始化GROUP_UID计数器
    if (!redis.Exists("group_uid_counter")) {
        redis.set("group_uid_counter", "0"); // 从1000开始计数
    }
    
    // 命令行参数检查
    if (argc != 3) {
        cerr << "Usage : " << argv[0] << " <IP> <PORT>" << endl;
        exit(EXIT_FAILURE);
    }

    // 创建服务器socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed!\n");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        cerr << "Invalid server IP " << endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    uint32_t port = atoi(argv[2]);
    server_addr.sin_port = htons(port);

    // 允许端口复用
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 增大接收缓冲区
    int buf_size = 2 * 1024 * 1024; // 2MB
    setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));

    // 绑定地址
    socklen_t server_len = sizeof(server_addr);
    if (bind(server_fd, (sockaddr*)&server_addr, server_len) < 0) {
        perror("bind failed!\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 开始监听
    if (listen(server_fd, LISTEN_NUM) < 0) {
        perror("listen failed!\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 设置非阻塞
    setnoblock(server_fd);
   

    // 创建线程池
    ThreadPool pool(1000);

    // 创建主Reactor（默认4个从Reactor）
    MainReactor main_reactor(server_fd, &pool, 4);

    // 启动从Reactor线程
    std::vector<std::thread> sub_threads;
    for (auto sub : main_reactor.get_sub_reactors()) {
        sub_threads.emplace_back([sub]() {
            sub->run();
        });
    }

    // 启动主Reactor（在主线程运行）
    cout << "服务器启动，主Reactor开始运行..." << endl;
    main_reactor.run();

    // 等待从Reactor线程结束（理论上不会执行到这里）
    for (auto& t : sub_threads) {
        t.join();
    }

    return 0;
}