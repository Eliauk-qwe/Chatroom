#include "server.hpp"
void friend_send_file(StickyPacket socket,Message &msg){
    recv_sendfile(socket,msg);
    //把消息存入服务器
    string notice1 = "我：发送了一个文件：" + msg.para[0];
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice1);

    string notice2 =msg.uid + ":发送了一个文件："+msg.para[0];
    redis.Rpush(msg.friend_or_group+"与"+msg.uid+"的聊天记录",notice2);

    //获取两个客户端的实时socket
    string fd1 =redis.Hget(msg.uid,"实时socket");
    string fd2 =redis.Hget(msg.friend_or_group,"实时socket");

    StickyPacket fd1_socket(stoi(fd1));
    StickyPacket fd2_socket(stoi(fd2));

    //回显给客户端1
    fd1_socket.mysend(notice1);
    

    //客户端2屏蔽了客户端1
    if(redis.sismember(msg.friend_or_group+"的屏蔽列表",msg.uid)){
        fd1_socket.mysend("你已被对方屏蔽");
        socket.mysend("have_been_quit");
        return;
    }

    //发送给客户端2
    if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") == msg.uid)){
        fd2_socket.mysend(notice2);
    }
    else if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") != msg.uid)){
        fd2_socket.mysend(msg.uid+"给你发了一个文件");
    }
    else if(online_users.find(msg.friend_or_group)==online_users.end()){
        string num=redis.Hget(msg.friend_or_group+"的未读消息","通知类消息");
        redis.hset(msg.friend_or_group+"的未读消息","通知类消息",to_string(stoi(num)));
        redis.Rpush(msg.friend_or_group+"的通知消息",msg.uid + "给你发来了一个文件");
    }


    socket.mysend("over");
    return;

}


void recv_sendfile(StickyPacket socket,Message &msg){
    string savepath = "./filesave/"+msg.friend_or_group+"/";

    int res=mkdir(savepath.c_str(),S_IRWXU | S_IRWXG | S_IRWXO);
    if (res != 0 && errno != EEXIST)
    {
        cerr << "Error creating directory: " << strerror(errno) << endl;
    }
    socket.mysend("ok");

    string filepath=savepath+msg.para[0];
    int fd =open(filepath.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
    if(fd<0){
        perror("open failed\n");
        close(fd);
        //socket.mysend("fail");
        return;
    }

    long long filesize=stoll(msg.para[2]);

    char buf[2048];
    long long sum=0;
    while(sum<filesize){
        int read_num=read(socket.getfd(),buf,2048);
        if(read_num==-1){
            if(errno==EAGAIN || errno==EWOULDBLOCK){
                continue;
            }else{
                perror("read failed\n");
                break;
            }
        }
        else{
            //cout << read_num <<endl;
            if(read_num==0){
                perror("客户端关闭连接\n");
                break;
            }
        }

        int write_num=write(fd,buf,read_num);
        if(read_num==-1){
            if(errno==EAGAIN || errno==EWOULDBLOCK){
                continue;
            }else{
                perror("read failed\n");
                break;
            }
        }
        sum=sum+write_num;
        cout << sum <<endl;
    }



    close(fd);
}


void friend_recv_file(StickyPacket socket,Message &msg){
    if(!send_sendfile(socket,msg)){
        socket.mysend("fail");
        return;
    }

    //把消息存入服务器
    string notice1 = "我：下载了一个文件：" + msg.para[0];
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice1);

    string notice2 =msg.uid + ":下载了一个文件："+msg.para[0];
    redis.Rpush(msg.friend_or_group+"与"+msg.uid+"的聊天记录",notice2);

    //获取两个客户端的实时socket
    string fd1 =redis.Hget(msg.uid,"实时socket");
    string fd2 =redis.Hget(msg.friend_or_group,"实时socket");

    StickyPacket fd1_socket(stoi(fd1));
    StickyPacket fd2_socket(stoi(fd2));

    //回显给客户端1
    fd1_socket.mysend(notice1);
    

    //客户端2屏蔽了客户端1
    if(redis.sismember(msg.friend_or_group+"的屏蔽列表",msg.uid)){
        fd1_socket.mysend("你已被对方屏蔽");
        socket.mysend("have_been_quit");
        return;
    }

    //发送给客户端2
    if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") == msg.uid)){
        fd2_socket.mysend(notice2);
    }
    else if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") != msg.uid)){
        fd2_socket.mysend(msg.uid+"下载了你发的一个文件");
    }
    else if(online_users.find(msg.friend_or_group)==online_users.end()){
        string num=redis.Hget(msg.friend_or_group+"的未读消息","通知类消息");
        redis.hset(msg.friend_or_group+"的未读消息","通知类消息",to_string(stoi(num)));
        redis.Rpush(msg.friend_or_group+"的通知消息",msg.uid + "给你发来了一个文件");
    }


    socket.mysend("over");
    return;

}


int send_sendfile(StickyPacket socket,Message &msg){
    string savepath = "./filesave/"+msg.friend_or_group+"/";
    string filepath=savepath+msg.para[0];

    int fd =open(filepath.c_str(),O_RDONLY);
    if(fd<0){
        perror("open failed!\n");
        close(fd);
        return 0;
    }
    else{
        struct stat statbuf;
        fstat(fd,&statbuf);
        long long filesize=statbuf.st_size;
        socket.mysend(to_string(filesize));

        off_t sum=0;
        while(sum<filesize){
            long long num=sendfile(socket.getfd(),fd,&sum,filesize-sum);
            if(num<0){
                if(errno==EAGAIN || errno==EWOULDBLOCK) continue;
                else{
                    perror("senfile() failed\n");
                    close(fd);
                    return 0;
                }
            }else if(num==0){
                perror("客户端关闭\n");
                close(fd);
                return 0;
            }
        }
    }

    close(fd);
    return 1;

    
}




void group_send_file(StickyPacket socket,Message &msg){
    recv_sendfile(socket,msg);
    //对于我
    string my_notice="我：发送了一个文件"+msg.para[0];
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",my_notice);
    string my_notice_fd=redis.Hget(msg.uid,"消息fd");
    StickyPacket my_notice_socket(stoi(my_notice_fd));
    //我肯定是群聊的在线用户，不需要特别颜色，也不需要处理未读消息
    my_notice_socket.mysend(my_notice);



    //对于他人
    string other_notice=msg.uid+":发送了一个文件"+msg.para[0];
    vector<string> groupmemberslist =redis.Smembers(msg.friend_or_group+"的群成员");
    for(const string &groupmember : groupmemberslist){
        if(groupmember==msg.uid)  continue;

        redis.Rpush(groupmember+"与"+msg.friend_or_group+"的聊天记录",other_notice);

        //对于登录着的人
        if(online_users.find(groupmember)!=online_users.end()){
            string other_notice_fd=redis.Hget(groupmember,"消息fd");
            StickyPacket other_notice_socket(stoi(other_notice_fd));

            //对于在这个群聊界面的人
            if(redis.sismember(msg.friend_or_group+"的在线用户",groupmember)){
                other_notice_socket.mysend(other_notice);
            }else{
                other_notice_socket.mysend(RED "群聊"+msg.friend_or_group+"发来了一条消息"+RESET);
            }



        }
        //对于未登录的人
        else{
            string num1=redis.Hget(groupmember+"的群聊消息",msg.friend_or_group);
            redis.hset(groupmember+"的群聊消息",msg.friend_or_group,to_string(stoi(num1)+1));

            //string num2=redis.Hget(msg.uid+"的未读消息","群聊消息");
            redis.hset(msg.uid+"的未读消息","群聊消息",to_string(stoi(num1)+1));
        }
    }

    socket.mysend("ok");

}
void group_recv_file(StickyPacket socket,Message &msg){
    if(!send_sendfile(socket,msg)){
        socket.mysend("fail");
        return;
    }

    //对于我
    string my_notice="我：接收了一个文件"+msg.para[0];
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",my_notice);
    string my_notice_fd=redis.Hget(msg.uid,"消息fd");
    StickyPacket my_notice_socket(stoi(my_notice_fd));
    //我肯定是群聊的在线用户，不需要特别颜色，也不需要处理未读消息
    my_notice_socket.mysend(my_notice);

    socket.mysend("ok");
    


}

