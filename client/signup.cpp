#include "client.h"

void sign_up(){
    string phone,pass,pass2;
    string name;
    printf("请输入电话号码：\n");
    getline(cin,phone);
    for(int i=3;i>0;i--){
        cout<<"请设置密码:"<<endl;
        getline(cin,pass);
        cout<<"请重新输入密码:"<<endl;
        getline(cin,pass2);
        if(pass!=pass2){
            if(i==1)  return;
            cout<<"两次密码不一致，请重新输入"<<endl;
            cout<<"你还有"<<i-1<<"次机会"<<endl;
        }
        else   break;
    }

    /*cout<<"请输入密保问题"<<endl;
    getline(cin,question);
    cout<<"请输入回答"<<endl;
    getline(cin,answer);*/

    cout<<"请设置昵称"<<endl;
    getline(cin,name);

    Message msg(SIGNUP,phone,pass,name);
    socket_fd.mysend(msg.S_to_json());
    //错误处理

    string uid=socket_fd.client_recv();
    if (uid == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    cout<<"你注册的uid为:"<<QING+uid+RESET<<endl;
    printf("请记住它哦，它是类似学号一样重要的东西\n");
    //cout<<"接下来将进入登录界面"<<endl<<endl;
    return;

}

int log_in(){
    string phone,pass,uid;
    int i=3;
    while(1){
        cout<<"请输入你的电话号码:"<<endl;
        getline(cin,phone);
        cout<<"请输入你的密码:"<<endl;
        getline(cin,pass);
        cout<<"请输入你的uid:"<<endl;
        getline(cin,uid);
        log_uid=uid;

        Message msg(phone,uid,pass,LOGIN);
        
        socket_fd.mysend(msg.S_to_json());

        string recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
        if(recv=="该用户未注册"){
            cout<<"你还未注册，请先注册"<<endl;
            return 0;
        }else if(recv=="该用户已登录"){
            cout<<"你已经登录，请勿重复登录"<<endl;
            return 0;
        }else if(recv=="密码错误"){
            cout<<"密码错误"<<endl;
            i--;
            if(i==0) return 0;
            cout<<"你还有"<<i<<"次机会"<<endl;
            continue;
        }else if(recv=="ok"){
            cout<<"登录成功"<<endl;
           
            // 修复点1：添加分号并重命名线程变量
            thread notice_thread([uid=log_uid,noticefd=socket_fd.get_notice_fd()](){
                notice_recv_thread(uid,noticefd);
            });
            notice_thread.detach();

            // 修复点2：添加分号并重命名线程变量
            thread heart_thread_1([uid=log_uid,fd=socket_fd.getfd()](){
                heartthread(uid,fd);
            });
            heart_thread_1.detach();

            /*thread heart_thread_2([uid=log_uid,fd=socket_fd.get_notice_fd()](){
                heartthread(uid,fd);
            });
            heart_thread_2.detach();*/


            
            return 1;
        }


    }
}

void notice_recv_thread(string uid,int noticefd){
    StickyPacket noticesocket(noticefd);

    
    sockaddr_in notice_addr=client_addr;
    if(connect(noticefd,(sockaddr*)&notice_addr,sizeof(notice_addr))  <0){
        perror("connect failed!\n");
        close(noticefd);
        return;
    }

    /*thread heart_thread_2([uid = log_uid, fd = socket_fd.get_notice_fd()]()
                          { heartthread(uid, fd); });
    heart_thread_2.detach();*/

    Message msg(uid,NOTICE);
    //noticesocket.mysend(msg.S_to_json());
    if (noticesocket.mysend(msg.S_to_json()) < 0) {
        cerr << "通知注册失败" << endl;
        return;
    }

    while(1){
        string recv =noticesocket.client_recv();
        if(recv =="读取消息头不完整"){
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }

        cout << recv <<endl;
    }
    return;


}


void pass_find(){
    string uid,phone;
    cout<<"请输入你的uid:"<<endl;
    getline(cin,uid);
    cout<<"请输入你的电话:"<<endl;
    getline(cin,phone);

    Message msg(uid,phone,PASSFIND);
    socket_fd.mysend(msg.S_to_json());

    string recv=socket_fd.client_recv();
    if (recv == "读取消息头不完整")
    {
        cout << "服务器关闭" << endl;
        exit(EXIT_SUCCESS);
    }
    if(recv=="no"){
        cout<<"你的uid与电话不匹配,无法找回密码"<<endl;
    }else if(recv=="yes"){
        string pass=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
        cout<< "你的密码是："<< pass<<endl;
    }

    return;

}




void client_quit(int fd){
    if(log_uid=="0"){
        close(fd);
        int notice_fd=socket_fd.get_notice_fd();
        close(notice_fd);
        printf("连接已关闭\n");
        printf("感谢使用聊天室，再见！\n");
        exit(EXIT_SUCCESS);
    }else{
        Message msg(log_uid,CLIENT_QUIT);
        socket_fd.mysend(msg.S_to_json());
        string recv=socket_fd.client_recv();
        if (recv == "读取消息头不完整")
        {
            cout << "服务器关闭" << endl;
            exit(EXIT_SUCCESS);
        }
        if(recv=="ok"){
            close(fd);
            int notice_fd=socket_fd.get_notice_fd();
            close(notice_fd);
            printf("连接已关闭\n");
            printf("感谢使用聊天室，再见！\n");
            exit(EXIT_SUCCESS);
        }
    }
}

void heartthread(string uid,int fd){
    //printf("心跳检测开始！\n");

   // string notice="heart";
   //int flag=true;

    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(10));

        Message msg(uid,HEART);

        socket_fd.mysend(msg.S_to_json());


    }

}