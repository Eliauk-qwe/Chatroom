#include <iostream>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <arpa/inet.h>

class HeartbeatServer {
public:
    HeartbeatServer(int port, int timeout_sec = 30)
        : port_(port), timeout_(std::chrono::seconds(timeout_sec)) {}
    
    bool start() {
        // 创建服务器socket
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            perror("socket creation failed");
            return false;
        }
        
        // 设置SO_REUSEADDR
        int opt = 1;
        if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            perror("setsockopt failed");
            return false;
        }
        
        // 绑定地址
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);
        
        if (bind(server_fd_, (sockaddr*)&address, sizeof(address)) < 0) {
            perror("bind failed");
            return false;
        }
        
        // 开始监听
        if (listen(server_fd_, 10) < 0) {
            perror("listen failed");
            return false;
        }
        
        std::cout << "Server listening on port " << port_ << "\n";
        
        // 创建epoll实例
        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ < 0) {
            perror("epoll creation failed");
            return false;
        }
        
        // 添加服务器socket到epoll
        add_to_epoll(server_fd_, EPOLLIN);
        
        // 启动心跳检测线程
        heartbeat_thread_ = std::thread(&HeartbeatServer::check_heartbeats, this);
        
        return true;
    }
    
    void run() {
        constexpr int MAX_EVENTS = 100;
        epoll_event events[MAX_EVENTS];
        
        while (running_) {
            int num_events = epoll_wait(epoll_fd_, events, MAX_EVENTS, 1000); // 1秒超时
            
            if (num_events < 0) {
                perror("epoll_wait error");
                continue;
            }
            
            for (int i = 0; i < num_events; ++i) {
                int fd = events[i].data.fd;
                
                if (fd == server_fd_) {
                    // 新连接
                    handle_new_connection();
                } else {
                    // 客户端事件
                    if (events[i].events & EPOLLIN) {
                        handle_client_data(fd);
                    }
                    
                    if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                        // 连接错误或挂起
                        remove_client(fd);
                    }
                }
            }
        }
    }
    
    void stop() {
        running_ = false;
        
        // 关闭所有连接
        {
            std::lock_guard lock(connections_mutex_);
            for (auto& [fd, _] : connections_) {
                close(fd);
            }
            connections_.clear();
        }
        
        // 等待心跳线程结束
        if (heartbeat_thread_.joinable()) {
            heartbeat_thread_.join();
        }
        
        close(server_fd_);
        close(epoll_fd_);
        std::cout << "Server stopped\n";
    }
    
private:
    void add_to_epoll(int fd, uint32_t events) {
        epoll_event ev{};
        ev.events = events;
        ev.data.fd = fd;
        
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
            perror("epoll_ctl add failed");
        }
    }
    
    void handle_new_connection() {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd_, (sockaddr*)&client_addr, &addr_len);
        
        if (client_fd < 0) {
            perror("accept failed");
            return;
        }
        
        // 设置非阻塞模式
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        
        // 添加到epoll
        add_to_epoll(client_fd, EPOLLIN | EPOLLET); // 边缘触发模式
        
        // 添加到连接映射
        {
            std::lock_guard lock(connections_mutex_);
            connections_[client_fd] = std::chrono::steady_clock::now();
        }
        
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
        std::cout << "New connection: " << ip_str << ":" << ntohs(client_addr.sin_port)
                  << " (fd: " << client_fd << ")\n";
    }
    
    void handle_client_data(int fd) {
        char buffer[1024];
        ssize_t bytes_read;
        bool heartbeat_received = false;
        
        while (true) {
            bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_read <= 0) {
                if (bytes_read == 0 || (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                    // 连接关闭或错误
                    remove_client(fd);
                }
                break;
            }
            
            buffer[bytes_read] = '\0';
            
            // 检查是否是心跳包
            if (strcmp(buffer, "HEARTBEAT") == 0) {
                heartbeat_received = true;
                // 更新心跳时间
                {
                    std::lock_guard lock(connections_mutex_);
                    if (connections_.find(fd) != connections_.end()) {
                        connections_[fd] = std::chrono::steady_clock::now();
                    }
                }
                std::cout << "Heartbeat received from fd " << fd << "\n";
            } else {
                // 处理其他数据
                std::cout << "Received data from fd " << fd << ": " << buffer << "\n";
            }
        }
    }
    
    void remove_client(int fd) {
        std::cout << "Removing client fd " << fd << "\n";
        
        // 从epoll移除
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
        
        // 从连接映射移除
        {
            std::lock_guard lock(connections_mutex_);
            connections_.erase(fd);
        }
        
        close(fd);
    }
    
    void check_heartbeats() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            auto now = std::chrono::steady_clock::now();
            std::vector<int> timed_out_clients;
            
            {
                std::lock_guard lock(connections_mutex_);
                for (const auto& [fd, last_heartbeat] : connections_) {
                    if (now - last_heartbeat > timeout_) {
                        timed_out_clients.push_back(fd);
                    }
                }
            }
            
            // 处理超时客户端
            for (int fd : timed_out_clients) {
                std::cout << "Client fd " << fd << " timed out\n";
                remove_client(fd);
            }
        }
    }
    
    int port_;
    int server_fd_ = -1;
    int epoll_fd_ = -1;
    std::chrono::seconds timeout_;
    std::thread heartbeat_thread_;
    std::atomic<bool> running_{true};
    
    // 连接映射: fd -> 最后心跳时间
    std::unordered_map<int, std::chrono::steady_clock::time_point> connections_;
    std::mutex connections_mutex_;
};

int main() {
    HeartbeatServer server(8080, 10); // 10秒超时
    
    if (!server.start()) {
        return 1;
    }
    
    // 在主线程运行事件循环
    server.run();
    
    server.stop();
    return 0;
}