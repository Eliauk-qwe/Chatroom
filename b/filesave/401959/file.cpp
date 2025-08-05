#include "server.hpp"
void friend_send_file(StickyPacket socket,Message &msg){
    //获取两个客户端的实时socket
    string fd1 =redis.Hget(msg.uid,"消息fd");
    string fd2 =redis.Hget(msg.friend_or_group,"消息fd");
    StickyPacket fd1_socket(stoi(fd1));
    StickyPacket fd2_socket(stoi(fd2));
    
    //对于他不是你的好友
    if(!redis.Hexists(msg.uid+"的好友列表",msg.friend_or_group)){
        socket.mysend("no");
        return;
    }


    //对于你被好友删除
    if((!redis.Hexists(msg.friend_or_group+"的好友列表",msg.uid)) &&(redis.Hexists(msg.uid+"的好友列表",msg.friend_or_group))){
        //socket.mysend("friend_del");
        string  notice="我:" RED "!" RESET "你已被好友删除，文件无法上传"; 
        fd1_socket.mysend(notice);
        redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice);
        socket.mysend("del");
        return;
    }


    //对于你被好友屏蔽
    if(redis.Hexists(msg.friend_or_group+"的屏蔽列表",msg.uid)){
        string  notice="我:" RED "!" RESET "你已被好友屏蔽，文件无法上传"; 
        fd1_socket.mysend(notice);
        redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice);
        socket.mysend("quit");
        return; 
    }


    int res=recv_sendfile(socket,msg);
    if(res<0){
        printf("服务器接受文件失败\n");
        return;
        
    }else if(res==0){
        printf("服务器接受文件成功\n");
        
    }


    redis.hset("文件ID-NAME表",msg.other,msg.para[0]);



    

    //正常聊天
    //把消息存入服务器
    string notice1 =PLUSWHITE "我：" RESET"上传了文件" + msg.para[0]+YELLOW+msg.other+RESET;
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice1);

    string name1=redis.Hget(msg.uid,"name");
    string notice2 =ZI+ name1 +RESET+ ":上传了文件" + msg.para[0]+YELLOW+msg.other+RESET;
    redis.Rpush(msg.friend_or_group+"与"+msg.uid+"的聊天记录",notice2);

    //对于你
    fd1_socket.mysend(notice1);

    //对于好友
    //发送给客户端2
    if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") == msg.uid)){
        fd2_socket.mysend(notice2);
    }
    else if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") != msg.uid)){
        fd2_socket.mysend(QING+msg.uid+":"+name1+"给你发了一个文件"+RESET);
        //总数量
        /*string num1=redis.Hget(msg.friend_or_group+"的未读消息","好友消息");
        redis.hset(msg.friend_or_group+"的未读消息","好友消息",to_string(stoi(num1)+1));
        //单个好友数量
        string num2=redis.Hget(msg.uid+"的好友消息",msg.friend_or_group);
        redis.hset(msg.uid+"的好友消息",msg.friend_or_group,to_string(stoi(num1)+1));*/
        
    }
    else if(online_users.find(msg.friend_or_group)==online_users.end()){
        //总数量
        /*string num1=redis.Hget(msg.friend_or_group+"的未读消息","好友消息");
        redis.hset(msg.friend_or_group+"的未读消息","好友消息",to_string(stoi(num1)+1));
        //单个好友数量
        string num2=redis.Hget(msg.uid+"的好友消息",msg.friend_or_group);
        redis.hset(msg.uid+"的好友消息",msg.friend_or_group,to_string(stoi(num1)+1));*/
        
    }
    
    return;


}





void friend_recv_file(StickyPacket socket,Message &msg){
   int res=send_sendfile(socket,msg);
    if(res<0){
        printf("服务器接受文件失败\n");
        return;
        
    }else if(res==0){
        printf("服务器接受文件成功\n");
        
    }

   //获取两个客户端的实时socket
    string fd1 =redis.Hget(msg.uid,"消息fd");
    string fd2 =redis.Hget(msg.friend_or_group,"消息fd");
    StickyPacket fd1_socket(stoi(fd1));
    StickyPacket fd2_socket(stoi(fd2));
    
    


    //对于你被好友删除
   if((!redis.Hexists(msg.friend_or_group+"的好友列表",msg.uid)) &&(redis.Hexists(msg.uid+"的好友列表",msg.friend_or_group))){
        
        /*string  notice="我:" RED "!" RESET "你已被好友删除，文件无法上传"; 
        fd1_socket.mysend(notice);
        redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice);
        socket.mysend("del");*/
        return;
    }


    //对于你被好友屏蔽
    if(redis.Hexists(msg.friend_or_group+"的屏蔽列表",msg.uid)){
        /*string  notice="我:" RED "!" RESET "你已被好友屏蔽，文件无法上传"; 
        fd1_socket.mysend(notice);
        redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice);
        socket.mysend("quit");*/
        return; 
    }

                                    
    //正常聊天
    //把消息存入服务器
    string notice1 =PLUSWHITE "我：" RESET"下载了文件" + msg.other;
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice1);

    string name1=redis.Hget(msg.uid,"name");
    string notice2 =QING+ name1+RESET + ":下载了文件" + msg.other;
    redis.Rpush(msg.friend_or_group+"与"+msg.uid+"的聊天记录",notice2);

    //对于你
    fd1_socket.mysend(notice1);

    //对于好友
    //发送给客户端2
    if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") == msg.uid)){
        fd2_socket.mysend(notice2);
    }
    else if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") != msg.uid)){
        fd2_socket.mysend(QING+msg.uid+":"+name1+"下载了你发的文件"+RESET);
        //总数量
        /*string num1=redis.Hget(msg.friend_or_group+"的未读消息","好友消息");
        redis.hset(msg.friend_or_group+"的未读消息","好友消息",to_string(stoi(num1)+1));
        //单个好友数量
        string num2=redis.Hget(msg.uid+"的好友消息",msg.friend_or_group);
        redis.hset(msg.uid+"的好友消息",msg.friend_or_group,to_string(stoi(num1)+1));*/
        
    }
     
    else if(online_users.find(msg.friend_or_group)==online_users.end()){
        //总数量
        /*string num1=redis.Hget(msg.friend_or_group+"的未读消息","好友消息");
        redis.hset(msg.friend_or_group+"的未读消息","好友消息",to_string(stoi(num1)+1));
        //单个好友数量
        string num2=redis.Hget(msg.uid+"的好友消息",msg.friend_or_group);
        redis.hset(msg.uid+"的好友消息",msg.friend_or_group,to_string(stoi(num1)+1));*/
        
    }
    
    return;


}

void group_send_file(StickyPacket socket,Message &msg){
    int res=recv_sendfile(socket,msg);
    if(res<0){
        printf("服务器接受文件失败\n");
    }else if(res==0){
        printf("服务器接受文件成功\n");
    }
     //对于我
    string groupname=redis.Hget("群聊ID-NAME表",msg.friend_or_group);
    string my_notice=PLUSWHITE"我：" RESET "发送了一个文件"+msg.para[0]+YELLOW+msg.other+RESET;
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+":"+groupname+"的聊天记录",my_notice);
    string my_notice_fd=redis.Hget(msg.uid,"消息fd");
    //cout<<my_notice_fd<<"1111111111"<<endl;
    StickyPacket my_notice_socket(stoi(my_notice_fd));
    //我肯定是群聊的在线用户，不需要特别颜色，也不需要处理未读消息
    my_notice_socket.mysend(my_notice);



    //对于他人
    string name1=redis.Hget(msg.uid,"name");
    string other_notice=name1+"("+msg.uid+"):发送了一个文件"+msg.para[0]+YELLOW+msg.other+RESET;
    vector<string> groupmemberslist =redis.Smembers(msg.friend_or_group+"的群成员");
   
    for(const string &groupmember : groupmemberslist){
        
        if(groupmember==msg.uid)  continue;

        redis.Rpush(groupmember+"与"+msg.friend_or_group+":"+groupname+"的聊天记录",other_notice);

        //对于登录着的人
        if(online_users.find(groupmember)!=online_users.end()){
            string other_notice_fd=redis.Hget(groupmember,"消息fd");
            cout<<other_notice_fd<<"222222"<<endl;

            
            StickyPacket other_notice_socket(stoi(other_notice_fd));
           


            //对于在这个群聊界面的人
            if(redis.sismember(msg.friend_or_group+"的在线用户",groupmember)){
                other_notice_socket.mysend(other_notice);
            }else{
                other_notice_socket.mysend(QING "群聊"+msg.friend_or_group+"发送了一个文件"+RESET);
               // string num1=redis.Hget(groupmember+"的群聊消息",msg.friend_or_group);
                
                //redis.hset(groupmember+"的群聊消息",msg.friend_or_group,to_string(stoi(num1)+1));

                //string num2=redis.Hget(msg.uid+"的未读消息","群聊消息");
                //redis.hset(msg.uid+"的未读消息","群聊消息",to_string(stoi(num1)+1));
            }



        }
        //对于未登录的人
        else{
            

            //string num1=redis.Hget(groupmember+"的群聊消息",msg.friend_or_group);
           
           // redis.hset(groupmember+"的群聊消息",msg.friend_or_group,to_string(stoi(num1)+1));

            //string num2=redis.Hget(msg.uid+"的未读消息","群聊消息");
            //redis.hset(msg.uid+"的未读消息","群聊消息",to_string(stoi(num1)+1));
        }
    }

    //socket.mysend("ok");


}
void group_recv_file(StickyPacket socket,Message &msg){
    int res=send_sendfile(socket,msg);
    if(res<0){
        printf("服务器发送文件失败\n");
    }else if(res==0){
        printf("服务器发送文件成功\n");
    }

    //cout<<"888888888888888"<<endl;

    //对于我
    string groupname=redis.Hget("群聊ID-NAME表",msg.friend_or_group);
    string my_notice=PLUSWHITE"我：" RESET "接受了一个文件"+msg.para[1];
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+":"+groupname+"的聊天记录",my_notice);
    string my_notice_fd=redis.Hget(msg.uid,"消息fd");
   // cout<<my_notice_fd<<"1111111111"<<endl;

    StickyPacket my_notice_socket(stoi(my_notice_fd));
    //我肯定是群聊的在线用户，不需要特别颜色，也不需要处理未读消息
    my_notice_socket.mysend(my_notice);

    //socket.mysend("ok");


    


}


int send_sendfile(StickyPacket socket,Message &msg){
    string filepath = "./filesave/"+msg.para[0]/*+"/"+msg.para[1]*/;

    int fd =open(filepath.c_str(),O_RDONLY);
    if(fd<0){
        perror("open failed!\n");
        socket.mysend("open fail");
        close(fd);
        return -1;
    }

    //获取文件大小
    off_t filesize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    if(filesize ==0){
        printf("你所发文件大小为0,无需发送");
        return -1;
    }
    socket.mysend(to_string(filesize));

    printf("Sending file: %s (Size: %ld bytes)\n", msg.other.c_str(), filesize);

    off_t offset = 0;
    ssize_t send_bytes;
    ssize_t total_send = 0;

    while (total_send < filesize)
    {
         // 计算本次要接收的最大字节数
        size_t remaining = filesize - total_send;
        size_t send_size = (remaining < 65536) ? remaining : 65536;

        send_bytes = sendfile(socket.getfd(), fd, &offset, send_size);
        if (send_bytes == -1)
        {
            fprintf(stderr, "sendfile错误:%s\n", strerror(errno));

            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else
            {
                close(fd);
                return -1;
            }
        }
        else if (send_bytes == 0)
        {
            perror("连接关闭\n");
            close(fd);
            return -1;
        }

        total_send += send_bytes;
        printf("Sent %zd bytes (%.2f%%)\n", send_bytes, (double)total_send / filesize * 100);
    }

    if (total_send == filesize)
    {
        printf("File sent successfully! Total bytes: %zd\n", total_send);
    }
    else
    {
        printf("File transfer incomplete. Sent %zd/%ld bytes\n", total_send, filesize);
    }

    close(fd);
    return 0;

    
}



int recv_sendfile(StickyPacket socket,Message &msg){
    // 确保基础目录存在
    mkdir("./filesave", 0755); // 忽略错误

    string savepath = "./filesave/"+msg.other+"/";
    
    //所有者读写执行，组和其他用户读执行
    int num=mkdir(savepath.c_str(),0755);

    if (num == -1 && errno != EEXIST) // 忽略"已存在"错误
    {
        fprintf(stderr, "创建目录 %s 失败：%s\n",savepath.c_str(), strerror(errno));
        socket.mysend("mkdir fail");
        return -1;
    }

    string filepath=savepath+msg.para[0];
    //cout<<filepath<<endl;
    int fd =open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if(fd<0){
        perror("open failed\n");
        close(fd);
        socket.mysend("open fail");
        return -1;
    }
    socket.mysend("ok");

    
    off_t filesize=(off_t)stoll(msg.para[1]);
    //cout<<"filesize"<<filesize<<endl;

    off_t total_recv=0;
    ssize_t recv_bytes;
    ssize_t write_bytes;
    vector<char> buf(65536);
    
    while(total_recv<filesize){
        // 计算本次要接收的最大字节数
        size_t remaining = filesize - total_recv;
        size_t recv_size = (remaining < 65536) ? remaining : 65536;


        
        recv_bytes=recv(socket.getfd(),buf.data(),recv_size,0);
        

        if (recv_bytes == -1)
        {
            //fprintf(stderr, "recv错误:%s\n", strerror(errno));

            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else
            {
                close(fd);
                return -1;
            }
        }
        else if (recv_bytes == 0)
        {
            perror("客户端连接关闭\n");
            close(fd);
            return -1;
        }
        
        write_bytes=write(fd,buf.data(),recv_bytes);
        
        if(write_bytes==-1){
            //fprintf(stderr, "recv错误:%s\n", strerror(errno));

            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else
            {
                close(fd);
                return -1;
            }
        }

        total_recv = total_recv + write_bytes;
        printf("Recv %zd bytes (%.2f%%)\n", recv_bytes, (double)total_recv / filesize * 100);
    }

    if (total_recv == filesize)
    {
        printf("File recv successfully! Total bytes: %zd\n", total_recv);
    }
    else
    {
        printf("File transfer incomplete. Recv %zd/%ld bytes\n", total_recv, filesize);
    }
    //cout<<filepath<<endl;

    close(fd);
    return 0;
}

