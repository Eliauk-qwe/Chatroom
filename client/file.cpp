#include "client.hpp"
#include <sys/socket.h>
int send_file(string uid,string friend_or_group,StickyPacket f_socket,int flag,string filepath){
    


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
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }

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
             // 计算本次要接收的最大字节数
            size_t remaining = filesize - total_send;
            size_t send_size = (remaining < 65536) ? remaining : 65536;

            

            send_bytes = sendfile(f_socket.getfd(), fd, &offset, send_size);
            

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


int friend_recv_file(string uid,StickyPacket f_socket,int flag,string friend_or_group,string filename,string want_path){

    
    string filepath=want_path+"/"+filename;

    Message msg(uid,friend_or_group,{friend_or_group,filename},flag);
    f_socket.mysend(msg.S_to_json());
    string res =f_socket.client_recv();
    if (res == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }

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
        //printf("Recv %zd bytes (%.2f%%)\n", recv_bytes, (double)total_recv / filesize * 100);
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


void sfile(string uid,string friend_or_group,int flag,string path){
    printf("11111111111111\n");
    int file_fd = socket(AF_INET,SOCK_STREAM,0);
    StickyPacket filesocket(file_fd);

    sockaddr_in file_addr=client_addr;
    if(connect(file_fd,(sockaddr*)&file_addr,sizeof(file_addr))  <0){
        perror("connect failed!\n");
        close(file_fd);
        return;
    }
    
    if(flag==FRIEND_SEND_FILE || flag == GROUP_SEND_FILE){
        int res = send_file(uid, friend_or_group,filesocket, FRIEND_SEND_FILE,path);
        if (res == 0)
        {
            printf("文件已成功上传至服务器\n");
        }
        else if (res < 0)
        {
            printf("文件未成功上传至服务器\n");
        }
    }
    

    close(file_fd);
}


void fvfile(string uid,string friend_or_group,int flag,string filename,string want_path){
    int file_fd = socket(AF_INET,SOCK_STREAM,0);
    StickyPacket filesocket(file_fd);

    sockaddr_in file_addr=client_addr;
    if(connect(file_fd,(sockaddr*)&file_addr,sizeof(file_addr))  <0){
        perror("connect failed!\n");
        close(file_fd);
        return;
    }

    int res = friend_recv_file(uid, filesocket, flag, friend_or_group, filename, want_path);
    if (res == 0)
    {
        printf("文件已成功下载\n");
    }
    else if (res < 0)
    {
        printf("文件未成功下载\n");
    }

    close(file_fd);

}


int group_recv_file(string uid,StickyPacket f_socket,int flag,string friend_or_group,string filename,string want_path,string other_uid){
    string filepath=want_path+"/"+filename;

    Message msg(uid,friend_or_group,{other_uid,filename},flag);
    f_socket.mysend(msg.S_to_json());
    string res =f_socket.client_recv();
    if (res == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }

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
        //printf("Recv %zd bytes (%.2f%%)\n", recv_bytes, (double)total_recv / filesize * 100);
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


void gvfile(string uid,string friend_or_group,int flag,string filename,string want_path,string other_uid){
    int file_fd = socket(AF_INET,SOCK_STREAM,0);
    StickyPacket filesocket(file_fd);

    sockaddr_in file_addr=client_addr;
    if(connect(file_fd,(sockaddr*)&file_addr,sizeof(file_addr))  <0){
        perror("connect failed!\n");
        close(file_fd);
        return;
    }

    int res = group_recv_file(uid, filesocket, flag, friend_or_group, filename, want_path,other_uid);
    if (res == 0)
    {
        printf("文件已成功下载\n");
    }
    else if (res < 0)
    {
        printf("文件未成功下载\n");
    }

    close(file_fd);

}

