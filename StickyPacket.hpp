#ifndef _StickyPacket_H_
#define _StickyPacket_H_

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>

using namespace std;

class StickyPacket{
private:
    int fd=-1;
    int notice_fd=-1;
    //string recv_buffer;

public:
    int getfd() {
        return fd;
    }

    int get_notice_fd(){
        return notice_fd;
    }

    StickyPacket(){
        fd=socket(AF_INET,SOCK_STREAM,0);
    }

    StickyPacket(int fd){
        this->fd=fd;
    }

    StickyPacket(string msg){
        if(msg=="start"){
            fd=socket(AF_INET,SOCK_STREAM,0);
            notice_fd=socket(AF_INET,SOCK_STREAM,0);
        }
    }


    ssize_t readn(int fd,void *buffer,size_t size){
        size_t left;
        ssize_t nread;
        size_t count=0;
        char *buf=(char*)buffer;

        
        left=size;

        while(left>0){
            nread=read(fd,buf,left);
            if(nread==0)  break;
            // 处理可恢复错误（被信号中断或非阻塞无数据）
            else if(nread<0){
                if(errno==EINTR || errno==EWOULDBLOCK)  continue;
                else               return -1;
            }

            left=left-nread;
            buf=buf+nread;
            count=count+nread;

        }

        return count;
    }

    int mysend(string message){
        //cout<<"message:"<<message<<endl;

        // 添加有效性检查
        if (fd <= 0 || fcntl(fd, F_GETFL) < 0)
        {
            cerr << "[" << __func__ << "] 无效的文件描述符: " << fd << endl;
            return -1;
        }
        int message_len=message.size();
        int head_len=htonl(message_len);

        //消息封装（消息头+消息体）
        char *msg =new char[message_len+4];
        memcpy(msg,&head_len,4);
        memcpy(msg+4,message.data(),message_len);

        const char *buf=msg;
        int left=message_len+4;
        int nsend;
        int count=0;

        while(left>0){
           // cout<<left<<endl;
           // printf("%s\n",buf);

            nsend=send(fd,buf,left,0);
            //cout<<nsend<<endl;
            if(nsend<0){
                if(errno==EINTR || errno==EWOULDBLOCK) continue;
                else{
                    close(fd);
                    delete [] msg;
                    perror("send failed()!");
                    return -1;
                }
            }
            else if(nsend==0)  break;

            left=left-nsend;
            buf=buf+nsend;
            count=count+nsend;

        }
        //close (fd);
        delete [] msg;
        return count;

    }

    //服务端接受函数
    int server_recv(int cfd,string &msg){
        uint32_t len=0;
        // 1. 读取长度头（严格检查4字节）
        if(readn(cfd,(char *)&len,4)  !=4){
            close(cfd);
            return -1;
        };
        len=ntohl(len);
        
        //char *buf=new char[len+1];
        msg.resize(len);
        int nread=readn(cfd,&msg[0],len);


        // 3. 处理读取结果
        if (nread != len) {
            //delete[] buf;  // 释放内存
            if (nread == 0) {
                cout <<to_string(cfd) << "断开连接" <<endl;
                close(cfd);  // 客户端断开
                return 0;
            }
        
            cout << "数据不完整，关闭连接" <<endl;
            close(cfd);      // 数据不完整，关闭连接
            return -1;       // 返回错误
        }

        
        return nread;
        
    }


    

    string client_recv(){
        uint32_t len=0;
        // 1. 读取长度头（严格检查4字节）
        if(readn(fd,(char *)&len,4)  !=4){
            close(fd);
            return "读取消息头不完整";
        };
        len=ntohl(len);
        
        char *buf=new char[len+1];
        int nread=readn(fd,buf,len);


        // 3. 处理读取结果
        if (nread != len) {
            delete[] buf;  // 释放内存
            if (nread == 0) {
                cout <<to_string(fd) << "断开连接" <<endl;
                close(fd);  // 客户端断开
                return "断开连接";
            }
        
            cout << "数据不完整，关闭连接" <<endl;
            close(fd);      // 数据不完整，关闭连接
            return "数据不完整，关闭连接";       // 返回错误
        }

        buf[len]='\0';
        //*msg=buf;//通过参数返回消息
        string msg(buf);
        return msg;
        
    }


   

    string Receive_client()
    {
        // 数据头
        int len = 0;
        char *buf = new char[4];
        int cnt = 4;
        char *pt = (char *)&len;
        
        while (cnt > 0)
        {
            int ret = recv(fd, pt, cnt, 0);
            if (ret == -1)
            {
                close(fd);
                perror("read error");
                exit(0);
            }
            else if (ret == 0)
            {
                cout << "连接已结束" << endl;
                close(fd);
                delete[] buf;
                return "close";
            }
            pt += ret;
            cnt -= ret;
        }
        len = ntohl(len);
        delete[] buf;
        buf = new char[len + 1];
        cnt = len;
        pt = buf;

        while (cnt > 0)
        {
            int ret = recv(fd, pt, cnt, 0);
            if (ret == -1)
            {
                close(fd);
                perror("recv failed");
                delete[] buf;
                exit(0);
            }
            else if (ret == 0)
            {
                cout << "连接已结束" << endl;
                close(fd);
                delete[] buf;
                return "close";
            }
            pt += ret;
            cnt -= ret;
        }
        buf[len] = '\0';
        string msg(buf);
        delete[] buf;

        return msg;
    }
};


#endif


