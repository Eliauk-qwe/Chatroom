#include "client.hpp"
#include <sys/socket.h>
int send_file(string uid,string friend_or_group,StickyPacket f_socket,int flag){
    string filepath;
    printf("请输入你要发送的文件的路径:\n");
    getline(cin,filepath);


    //获取文件名字
    size_t lastpos = filepath.find_last_of("/");
    string filename = filepath.substr(lastpos + 1);

    //printf("现在开始上传文件\n");

    //以只读的方式打开文件
    int fd=open(filepath.c_str(),O_RDONLY);
    if(fd<0){
        perror("open failed()\n");
        close(fd);
        return -1;
    }

    //获取文件大小
    off_t filesize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    if(filesize ==0){
        printf("你所发文件大小为0,无需发送");
        close(fd);
        return -1;
    }

    
    
    Message msg(uid,friend_or_group,{filename, to_string(filesize)}, flag);
    f_socket.mysend(msg.S_to_json());
    string recv = f_socket.client_recv();

    if(recv=="no"){
        printf("你没有添加对方为好友\n");
        close(fd);
        return -1;
    }

    if (recv == "del")
    {
        printf("你已被好友删除\n");
        close(fd);
        return -1;
    }
    else if (recv == "quit")
    {
        printf("你已被好友屏蔽\n");
        close(fd);
        return -1;
    }

   
   
    if(recv=="mkdir fail"){
        printf("服务器创建目录失败\n");
        return -1;
    }

    if(recv=="open fail"){
        printf("服务器打开文件失败\n");
        return -1;
    }
   



    //服务器目录创建成功,并成功打开文件
    if (recv == "ok")
    {
        
        printf("Sending file: %s (Size: %ld bytes)\n", filename.c_str(), filesize);

        off_t offset = 0;
        ssize_t send_bytes;
        ssize_t total_send = 0;
        
        
        while (total_send < filesize)
        {
            

            send_bytes = sendfile(f_socket.getfd(), fd, &offset, filesize - offset);
            

            if (send_bytes ==-1)
            {
                fprintf(stderr,"sendfile错误:%s\n",strerror(errno));

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
                perror("服务端连接关闭\n");
                close(fd);
                return -1;
            }


            total_send += send_bytes;
            printf("Sent %zd bytes (%.2f%%)\n", send_bytes, (double)total_send / filesize * 100);
        }


        if (total_send == filesize) {
            printf("File sent successfully! Total bytes: %zd\n", total_send);
        } else {
            printf("File transfer incomplete. Sent %zd/%ld bytes\n", total_send, filesize);
        }
    }
    close(fd);

    

    return 0;
}


int recv_file(string uid,StickyPacket f_socket,int flag,string friend_or_group){
    string other_uid;
    if(flag==GROUP_RECV_FILE){
        printf("你要下载的文件的人的uid为\n");
        
        getline(cin,other_uid);
    }
    

    printf("你要下载的文件名为：\n");
    string filename;
    getline(cin,filename);

    printf("请输入你想存储的文件的位置（无需以 / 结尾）：\n");
    string want_path;
    getline(cin,want_path);

    string filepath=want_path+"/"+filename;

    Message msg(uid,friend_or_group,{other_uid,filename},flag);
    f_socket.mysend(msg.S_to_json());
    string res =f_socket.client_recv();

    int fd = open(filepath.c_str(), O_TRUNC | O_WRONLY | O_CREAT, S_IRWXU);
    if (fd < 0)
    {
        perror("open failed\n");
        close(fd);
        return -1;
    }

    if(res=="open fail"){
        printf("服务器打开文件失败\n");
        close(fd);
        return -1;
    }


    
    off_t filesize = (off_t)stoll(res);
    off_t total_recv = 0;
    ssize_t recv_bytes;
    ssize_t write_bytes;
    vector<char> buf(65536);
    
    while (total_recv < filesize)
    {
        // 计算本次要接收的最大字节数
        size_t remaining = filesize - total_recv;
        size_t recv_size = (remaining < 65536) ? remaining : 65536;

        recv_bytes=recv(f_socket.getfd(),buf.data(),recv_size,0);
        if (recv_bytes == -1)
        {
            fprintf(stderr, "recv错误:%s\n", strerror(errno));

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

        write_bytes = write(fd, buf.data(), recv_bytes);
        if (write_bytes == -1)
        {
            fprintf(stderr, "recv错误:%s\n", strerror(errno));

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
        printf("File sent successfully! Total bytes: %zd\n", total_recv);
    }
    else
    {
        printf("File transfer incomplete. Sent %zd/%ld bytes\n", total_recv, filesize);
    }
       
       
    close(fd);
    
    return 0;
}
