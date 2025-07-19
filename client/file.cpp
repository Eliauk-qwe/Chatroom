#include "client.hpp"
void send_file(string uid,string friend_uid,StickyPacket f_socket,int flag){
    string filepath;
    printf("请输入你要发送的文件的路径:\n");
    getline(cin,filepath);

    int fd=open(filepath.c_str(),O_RDONLY);
    if(fd<0){
        perror("open failed()\n");
        close(fd);
        return;
    }else{
        off_t filesize=lseek(fd,0,SEEK_END);
        lseek(fd,0,SEEK_SET);
        size_t lastpos=filepath.find_last_of("/");
        string filename = filepath.substr(lastpos+1);

        
        Message msg(uid,friend_uid,{filename,to_string(filesize)},flag);
        f_socket.mysend(msg.S_to_json());

        string recv=f_socket.client_recv();
        if(recv== "ok"){
            ssize_t start=0;
            while(start<filesize){
                ssize_t num= sendfile(f_socket.getfd(),fd,&start,filesize-start);
                if(num==-1){
                    if(errno==EAGAIN || errno == EWOULDBLOCK) continue;
                    else{
                        perror("senfile failed\n");
                        close(fd);
                        return;
                    }
                }else if(num==0){
                    perror("连接关闭\n");
                    close(fd);
                    return;
                }

            }
            close(fd);
        }

        
    }
}


void recv_file(string uid,string friend_uid,StickyPacket f_socket,int flag){
    printf("你要下载的文件名为：\n");
    string filename;
    getline(cin,filename);

    printf("请输入你想存储的文件的位置：\n");
    string want_path;
    getline(cin,want_path);

    string filepath=want_path+"/"+filename;

    Message msg(uid,friend_uid,{filename},flag);
    f_socket.mysend(msg.S_to_json());

    string recv =f_socket.client_recv();
    /*if(recv==){

    }
    else{*/
        int fd =open(filepath.c_str(),O_APPEND | O_WRONLY | O_CREAT, S_IRWXU);
        if(fd<0){
            perror("open failed\n");
            close(fd);
            return;
        }

///////////////////////////////////////////////////////////////////////
       // cout << recv_file.c_str() << endl;
       long long filesize=stoll(recv);

       long long sum=0;
       char buf[2048];
       cout<< "文件大小为： "<< filesize<<endl;
       while(sum<filesize){
            int read_num=read(f_socket.getfd(),buf,2048);
            if(read_num<0){
                if(errno==EAGAIN || errno==EWOULDBLOCK) continue;
                else{
                    perror("read failed\n");
                    close(fd);
                    return;
                }
            }
            if(read_num==0){
                perror("客户端关闭\n");
                close(fd);
                return;
            }

            int write_num=write(fd,buf,read_num);
            if(write_num!=read_num){
                perror("no same\n");
            }
            if(write_num<0){
                if(errno==EAGAIN || errno==EWOULDBLOCK) continue;
                perror("write failed\n");
                close(fd);
                return;
            }

            sum=sum+write_num;

            
       }

       if(sum<filesize){
            perror("no same\n");
          
       }

       close(fd);

       printf("文件下载成功\n");

    



}
