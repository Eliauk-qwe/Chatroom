#include "server.hpp"
#include <csignal>

//int user_uid=1001;


MessageTrans trans;
Redis redis;
unordered_set<string> online_users;
std::unordered_map<int, std::chrono::steady_clock::time_point> heart_time;
std::chrono::seconds maxtime=std::chrono::seconds(60);





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




    // 在main函数中初始化UID计数器
    if (!redis.Exists("user_uid_counter"))
    {
        redis.set("user_uid_counter", "1"); // 从1000开始计数
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

   cout << "服务器开始工作" << endl;

   heart(epoll_fd);

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
                //sp_fd.server_recv(fd,client_cmd);
                /*if(sp_fd.server_recv(fd,client_cmd) <=0){
                    //没有接受到客户端任何信息
                    
                    cout<<client_cmd<<endl;

                    cout<<"5555555555555hhhhhhhhhhhh"<<endl;
                    close(fd);
                    continue;
                    //exit(EXIT_SUCCESS);
                }*/
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
                        perror("接收数据错误");
                    }

                    // 从 epoll 中移除
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);

                    // 关闭连接
                    close(fd);

                    // 如果是通知连接，清理相关资源
                    if (redis.Exists("fd-uid表"))
                    {
                        string uid = redis.Hget("fd-uid表", to_string(fd));
                        if (!uid.empty())
                        {
                            redis.hset(uid, "消息fd", "-1");
                            online_users.erase(uid);
                        }
                        redis.Hdel("fd-uid表", to_string(fd));
                    }

                    continue; // 继续处理下一个事件
                }
                cout<<client_cmd<<endl;


                //buf中为客户端请求的命令
                //string client_cmd = buf;
                /*cout << "客户端" << " "<< fd << "" << "的新请求为" << client_cmd <<endl
                     <<endl;*/

                Message msg;
                msg.Json_to_s(client_cmd);
                cout << msg.flag <<endl;
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
                else if(msg.flag==HEART){
                    heart_time[fd]=std::chrono::steady_clock::now();
                }
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

void heart(int epd){

    while (1)
    {
        auto now = std::chrono::steady_clock::now();

        vector<int> time_out_clients;

        if (heart_time.empty())
        {
            continue;
        }

        for (const auto &[fd, last_heartbeat] : heart_time)
        {
            if (now - last_heartbeat > maxtime)
            {
                time_out_clients.push_back(fd);
            }
        }

        for (int fd : time_out_clients)
        {
            cout << "客户端" << fd << "断开" << endl;
            epoll_ctl(epd, EPOLL_CTL_DEL, fd, nullptr);
            heart_time.erase(fd);
            close(fd);
        }
    }
}