#include "server.hpp"
#include <csignal>

//int user_uid=1001;


MessageTrans trans;
Redis redis;
unordered_set<string> online_users;
std::unordered_map<int, std::chrono::steady_clock::time_point> heart_time;
std::chrono::seconds maxtime=std::chrono::seconds(60);
// 推荐使用 constexpr 定义常量
//constexpr std::chrono::seconds HEARTBEAT_TIMEOUT = 60s;
std::mutex heart_mutex;






// 设置文件描述符为非阻塞模式
void setnoblock(int fd){
    int flag =fcntl(fd,F_GETFL);
     if(flag<0){
        cerr << "fcntl" <<endl;
        exit(EXIT_FAILURE);
    }

    if(fcntl(fd,F_SETFL,flag|O_NONBLOCK)  <0){
        cerr << "fcntl" <<endl;
        exit(EXIT_FAILURE);
    }
}





int  main(int argc,char *argv[]){
    signal(SIGPIPE, SIG_IGN);

    // 在main函数中初始化USER_UID计数器
    if (!redis.Exists("user_uid_counter"))
    {
        redis.set("user_uid_counter", "0");
    }


    // 在main函数中初始化GROUP_UID计数器
    if (!redis.Exists("group_uid_counter"))
    {
        redis.set("group_uid_counter", "0"); 
    }
    //再命令行要输入指定
    if(argc !=3){
        cerr << "Usage : " << argv[0] << "<IP> <PORT>" << endl;
        exit(EXIT_FAILURE);
    }

    
    int server_fd=socket(AF_INET,SOCK_STREAM,0) ;
    if(server_fd<0){
        perror("socked failed!\n");
    }


    //完成S 端均支持在 Web 自行指定 IP:Port
    //设置默认服务端IP（初始化）
    sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    //server_addr.sin_addr.s_addr=htonl(argv[1]);
    if(inet_pton(AF_INET,argv[1],(sockaddr*)&server_addr.sin_addr)  <=0){
        cerr << "Invailed  server IP " << endl;
        close(server_fd);
        exit(EXIT_FAILURE);

    }
    uint32_t port=atoi(argv[2]);
    server_addr.sin_port=htons(port); //htonl->htons

    //允许端口复用
    int opt=1;
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    // 增大接收缓冲区
    int buf_size = 2 * 1024 * 1024; // 2MB
    setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));
 
    socklen_t server_len =sizeof(server_addr);
    if(bind(server_fd,(sockaddr*)&server_addr,server_len) <0){
        perror("bind failed!\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd,LISTEN_NUM) <0){
        perror("listen failed!\n");
        close(server_fd);
        exit(EXIT_FAILURE);

    }
   

   
   //init_friend_message_process();


    //使用epoll
    int epoll_fd=epoll_create1(0);
    if(epoll_fd < 0){
        perror("epoll_create1\n");
        exit(EXIT_FAILURE);
    }

    setnoblock(server_fd);
    struct epoll_event ev;
    ev.data.fd=server_fd;
    ev.events = EPOLLIN | EPOLLET;
    //将文件描述符（FD）注册到 epoll 实例中，监控指定事件。
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_fd,&ev) < 0 ){
        perror("epoll_ctl\n");
        exit(EXIT_FAILURE);
    }
    /*
    struct epoll_event {
        uint32_t     events;  // 事件掩码 (EPOLLIN | EPOLLOUT | ...)
        epoll_data_t data;    // 用户数据
    };

    typedef union epoll_data {
        void    *ptr;   // 自定义数据结构指针
        int      fd;    // 关联的文件描述符
        uint32_t u32;
        uint64_t u64;
    } epoll_data_t;

    */


   //线程池，创建10个线程
   ThreadPool pool(10);

  // cout << "服务器开始工作" << endl;

   // 在main函数中启动心跳线程
   /*std::thread heart_thread(heart, epoll_fd);
   heart_thread.detach(); // 分离线程，使其独立运行*/

   //======================================================================
   //心跳检测
   //======================================================================
   
   
   // 存储就绪事件的数组
   struct epoll_event events[MAX_EVENTS];
   while(1){
        // 等待事件发生（-1表示无限等待），返回值为就绪事件数量
        int  count = epoll_wait(epoll_fd,events,MAX_EVENTS,-1); 
        if(count < 0){
            perror("epoll_wait\n");
            exit(EXIT_FAILURE);
        }

        cout << "Epoll returned " << count << " events" << endl;

        for(int i=0;i<count;i++){
            int fd = events[i].data.fd;
              
            //i=0,服务端接受客户端的连接,新客户端连接  
            if(fd==server_fd){
                sockaddr_in client_addr;
                socklen_t client_len =sizeof(client_addr);
                int new_fd=accept(server_fd,(sockaddr*)&client_addr,&client_len);
                if (new_fd == -1) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("accept");
                    }
                }
                printf("New connection: socket %d\n", new_fd);


                setnoblock(new_fd);
                struct epoll_event client_ev;
                client_ev.data.fd=new_fd;
                client_ev.events=EPOLLIN | EPOLLET;
                if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,new_fd,&client_ev) < 0 ){
                    perror("epoll_ctl\n");
                    exit(EXIT_FAILURE);
                }

            }
           
            //开始干活啦！
            else if(events[i].events & EPOLLIN){
                StickyPacket sp_fd(fd);
                string client_cmd;
               // 修改后：仅关闭当前连接
                int recv_ret = sp_fd.server_recv(fd, client_cmd);
                if (recv_ret <= 0)
                {
                    if (recv_ret == 0)
                    {
                        cout << "客户端关闭连接: " << fd << endl;
                    }
                    else
                    {
                       // perror("接收数据错误");
                    }

                    // 从 epoll 中移除
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    if(redis.Exists("客户端fd与对应uid表")){
                        string uid=redis.Hget("客户端fd与对应uid表",to_string(fd));
                        redis.Hdel("客户端fd与对应uid表",to_string(fd));
                        if(online_users.find(uid)!=online_users.end()){
                            online_users.erase(uid);
                        }
                        
                    }
                    cout << "客户端" << fd << "已断开连接" << endl;
                    // 增加心跳记录的清理
                    {
                        std::lock_guard<std::mutex> lock(heart_mutex);
                        heart_time.erase(fd);
                    }

                    // 关闭连接
                    close(fd);

                    continue; // 继续处理下一个事件
                }

                
                cout<<client_cmd<<endl;


                

                Message msg;
                msg.Json_to_s(client_cmd);
                //cout << msg.flag <<endl;
                if(msg.flag==NOTICE){
                    redis.hset(msg.uid, "消息fd", to_string(fd));
                }
                else if(msg.flag==FRIEND_SEND_FILE || msg.flag == FRIEND_RECV_FILE ||msg.flag==GROUP_SEND_FILE||msg.flag==GROUP_RECV_FILE){
                    ev.data.fd=fd;
                    ev.events=EPOLLIN | EPOLLET;
                    setnoblock(fd);
                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,&ev);
                    thread filethread([sp_fd,client_cmd,fd,epoll_fd](){
                        trans.translation(sp_fd,client_cmd);
                        struct epoll_event aev;
                        aev.data.fd=fd;
                        aev.events=EPOLLIN | EPOLLET;
                        epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&aev);
                    });

                    filethread.detach();
                }
                /*else if(msg.flag==HEART){
                    {
                    std::lock_guard<std::mutex> lock(heart_mutex);
                    heart_time[fd] = std::chrono::steady_clock::now();
                    }
                    redis.hset("客户端fd与对应uid表",to_string(fd),msg.uid);
                    //cout<<"收到客户端"<<fd<<",账号为"<<msg.uid<<"的心跳包"<<endl;;
                    cout<<RED "心跳" RESET<<endl;
                }*/
                /*else if (msg.flag == HEART)
                {
                    std::lock_guard<std::mutex> lock(heart_mutex); // ✅ 加锁保护
                    heart_time[fd] = std::chrono::steady_clock::now();
                    redis.hset("客户端fd与对应uid表", to_string(fd), msg.uid);
                    //online_users.insert(msg.uid); // ✅ 添加到在线用户
                    cout <<  RED "心跳" RESET << endl;
                }*/
                else{  
                    StickyPacket socket(fd); 
                    Task task([socket,client_cmd](){
                        trans.translation(socket,client_cmd);
                    });    
                    pool.addTask(task);

                    
                }
            }
        }

   }

}

/*void heart(int epd)
{

    
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto now = std::chrono::steady_clock::now();

        vector<int> time_out_clients;
        {
            std::lock_guard<std::mutex> lock(heart_mutex);

            for (const auto &[fd, last_heartbeat] : heart_time)
            {
                if (now - last_heartbeat > maxtime)
                {
                    time_out_clients.push_back(fd);
                }
            }
        }

            for (int fd : time_out_clients)
            {

                {
                    std::lock_guard<std::mutex> lock(heart_mutex);
                    close(fd);
                    epoll_ctl(epd, EPOLL_CTL_DEL, fd, nullptr);
                    if (redis.Exists("客户端fd与对应uid表"))
                    {
                        string uid = redis.Hget("客户端fd与对应uid表", to_string(fd));
                        redis.Hdel("客户端fd与对应uid表", to_string(fd));
                        if(online_users.find(uid)!=online_users.end()){
                            online_users.erase(uid);
                        }
                    }
                }

                cout << "客户端" << fd << "已断开连接" << endl;
                time_out_clients.erase(fd);
               
            }
        
    
}*/



// 修复后的心跳线程函数
/*void heart(int epd)
{
    while (true) // ✅ 改为无限循环
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto now = std::chrono::steady_clock::now();

        vector<int> time_out_clients;
        {
            std::lock_guard<std::mutex> lock(heart_mutex);
            // 遍历并找出超时的客户端
            for (auto it = heart_time.begin(); it != heart_time.end();)
            {
                if (now - it->second > maxtime)
                {
                    time_out_clients.push_back(it->first);
                    it = heart_time.erase(it); // ✅ 从记录中移除
                }
                else
                {
                    ++it;
                }
            }
        }

        // 处理超时客户端
        for (int fd : time_out_clients)
        {
            close(fd);
            epoll_ctl(epd, EPOLL_CTL_DEL, fd, nullptr);
            
            // 清理Redis和在线用户记录
            if (redis.Exists("客户端fd与对应uid表"))
            {
                string uid = redis.Hget("客户端fd与对应uid表", to_string(fd));
                redis.Hdel("客户端fd与对应uid表", to_string(fd));
                
                std::lock_guard<std::mutex> lock(heart_mutex);
                if (online_users.find(uid) != online_users.end())
                {
                    online_users.erase(uid);
                }
            }
            cout << "客户端" << fd << "已断开连接（心跳超时）" << endl;
        }
    }
}*/



// #include "server.hpp"
// #include <csignal>
// #include <functional>

// MessageTrans trans;
// Redis redis;
// unordered_set<string> online_users;
// std::unordered_map<int, std::chrono::steady_clock::time_point> heart_time;
// constexpr std::chrono::seconds HEARTBEAT_TIMEOUT(60);
// std::mutex heart_mutex;

// // 设置文件描述符为非阻塞模式
// void setnoblock(int fd) {
//     int flag = fcntl(fd, F_GETFL);
//     if (flag < 0) {
//         cerr << "fcntl F_GETFL error: " << strerror(errno) << endl;
//         exit(EXIT_FAILURE);
//     }

//     if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) < 0) {
//         cerr << "fcntl F_SETFL error: " << strerror(errno) << endl;
//         exit(EXIT_FAILURE);
//     }
// }

// // 安全关闭连接
// void close_connection(int fd, int epoll_fd, Redis& redis) {
//     // 从epoll中移除
//     epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    
//     // 清理Redis记录
//     if (redis.Exists("客户端fd与对应uid表")) {
//         string uid = redis.Hget("客户端fd与对应uid表", to_string(fd));
//         redis.Hdel("客户端fd与对应uid表", to_string(fd));
        
//         // 从在线用户中移除
//         std::lock_guard<std::mutex> lock(heart_mutex);
//         if (!uid.empty() && online_users.find(uid) != online_users.end()) {
//             online_users.erase(uid);
//         }
//     }
    
//     // 清理心跳记录
//     {
//         std::lock_guard<std::mutex> lock(heart_mutex);
//         heart_time.erase(fd);
//     }
    
//     // 关闭文件描述符
//     close(fd);
//     cout << "客户端" << fd << "已断开连接" << endl;
// }

// // 心跳检测函数
// void check_heartbeats(int epoll_fd, Redis& redis) {
//     auto now = std::chrono::steady_clock::now();
//     std::vector<int> timeout_fds;
    
//     // 收集超时连接
//     {
//         std::lock_guard<std::mutex> lock(heart_mutex);
//         for (auto& [fd, last_time] : heart_time) {
//             if (now - last_time > HEARTBEAT_TIMEOUT) {
//                 timeout_fds.push_back(fd);
//             }
//         }
//     }
    
//     // 关闭超时连接
//     for (int fd : timeout_fds) {
//         close_connection(fd, epoll_fd, redis);
//     }
// }

// int main(int argc, char *argv[]) {
//     signal(SIGPIPE, SIG_IGN);

//     // 初始化计数器
//     if (!redis.Exists("user_uid_counter")) {
//         redis.set("user_uid_counter", "1000");
//     }
//     if (!redis.Exists("group_uid_counter")) {
//         redis.set("group_uid_counter", "1000"); 
//     }
    
//     if (argc != 3) {
//         cerr << "Usage: " << argv[0] << " <IP> <PORT>" << endl;
//         exit(EXIT_FAILURE);
//     }

//     int server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_fd < 0) {
//         perror("socket failed");
//         exit(EXIT_FAILURE);
//     }

//     sockaddr_in server_addr;
//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
    
//     if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
//         cerr << "Invalid server IP" << endl;
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }
    
//     uint32_t port = atoi(argv[2]);
//     server_addr.sin_port = htons(port);

//     // 设置套接字选项
//     int opt = 1;
//     setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
//     int buf_size = 2 * 1024 * 1024; // 2MB
//     setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));

//     // 绑定和监听
//     if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
//         perror("bind failed");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }

//     if (listen(server_fd, LISTEN_NUM) < 0) {
//         perror("listen failed");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }

//     init_friend_message_process();

//     // 创建epoll实例
//     int epoll_fd = epoll_create1(0);
//     if (epoll_fd < 0) {
//         perror("epoll_create1");
//         exit(EXIT_FAILURE);
//     }

//     setnoblock(server_fd);
//     struct epoll_event ev;
//     ev.data.fd = server_fd;
//     ev.events = EPOLLIN | EPOLLET;
//     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
//         perror("epoll_ctl");
//         exit(EXIT_FAILURE);
//     }

//     // 创建线程池
//     ThreadPool pool(10);

//     // 启动心跳检测线程（周期性任务）
//     std::thread([&]() {
//         while (true) {
//             std::this_thread::sleep_for(std::chrono::seconds(30));
//             pool.addTask([epoll_fd, &redis]() {
//                 check_heartbeats(epoll_fd, redis);
//             });
//         }
//     }).detach();

//     // 主事件循环
//     struct epoll_event events[MAX_EVENTS];
//     while (true) {
//         int count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
//         if (count < 0) {
//             if (errno == EINTR) continue; // 被信号中断
//             perror("epoll_wait");
//             exit(EXIT_FAILURE);
//         }

//         for (int i = 0; i < count; i++) {
//             int fd = events[i].data.fd;
            
//             // 新连接
//             if (fd == server_fd) {
//                 // ET模式需要循环accept
//                 while (true) {
//                     sockaddr_in client_addr;
//                     socklen_t client_len = sizeof(client_addr);
//                     int new_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
                    
//                     if (new_fd == -1) {
//                         if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                             break; // 所有连接已处理
//                         }
//                         perror("accept");
//                         continue;
//                     }
                    
//                     char client_ip[INET_ADDRSTRLEN];
//                     inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
//                     printf("新连接: socket %d, IP: %s, 端口: %d\n", 
//                            new_fd, client_ip, ntohs(client_addr.sin_port));

//                     setnoblock(new_fd);
//                     struct epoll_event client_ev;
//                     client_ev.data.fd = new_fd;
//                     client_ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                    
//                     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &client_ev) < 0) {
//                         perror("epoll_ctl add client");
//                         close(new_fd);
//                     }
//                 }
//             }
//             // 连接关闭
//             else if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
//                 close_connection(fd, epoll_fd, redis);
//             }
//             // 数据可读
//             else if (events[i].events & EPOLLIN) {
//                 StickyPacket sp_fd(fd);
//                 string client_cmd;
//                 int recv_ret = sp_fd.server_recv(fd, client_cmd);
                
//                 if (recv_ret <= 0) {
//                     if (recv_ret == 0) {
//                         cout << "客户端关闭连接: " << fd << endl;
//                     }
//                     close_connection(fd, epoll_fd, redis);
//                     continue;
//                 }
                
//                 Message msg;
//                 try {
//                     msg.Json_to_s(client_cmd);
                    
//                     // 心跳包处理
//                     if (msg.flag == HEART) {
//                         std::lock_guard<std::mutex> lock(heart_mutex);
//                         heart_time[fd] = std::chrono::steady_clock::now();
                        
//                         if (!msg.uid.empty()) {
//                             redis.hset("客户端fd与对应uid表", to_string(fd), msg.uid);
//                             online_users.insert(msg.uid);
//                         }
//                         continue;
//                     }
                    
//                     // 文件传输任务
//                     if (msg.flag == FRIEND_SEND_FILE || msg.flag == FRIEND_RECV_FILE || 
//                         msg.flag == GROUP_SEND_FILE || msg.flag == GROUP_RECV_FILE) {
                        
//                         // 从epoll中临时移除
//                         struct epoll_event temp_ev;
//                         temp_ev.data.fd = fd;
//                         temp_ev.events = EPOLLIN | EPOLLET;
//                         epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &temp_ev);
                        
//                         // 提交到线程池
//                         pool.addTask([sp_fd, client_cmd, fd, epoll_fd]() {
//                             trans.translation(sp_fd, client_cmd);
                            
//                             // 重新加入epoll
//                             struct epoll_event aev;
//                             aev.data.fd = fd;
//                             aev.events = EPOLLIN | EPOLLET;
//                             epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &aev);
//                         });
//                     }
//                     // 普通任务
//                     else {
//                         pool.addTask([sp_fd, client_cmd]() {
//                             trans.translation(sp_fd, client_cmd);
//                         });
//                     }
//                 } catch (const std::exception& e) {
//                     cerr << "消息处理异常: " << e.what() << endl;
//                 }
//             }
//         }
//     }
    
//     close(server_fd);
//     return 0;
// }