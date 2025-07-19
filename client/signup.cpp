#include "client.hpp"

void sign_up(){
    string phone,pass,pass2;
    string question,answer;
    string name;
    printf("请输入电话号码：\n");
    getline(cin,phone);
    for(int i=5;i>0;i--){
        cout<<"请设置密码"<<endl;
        getline(cin,pass);
        cout<<"请重新输入密码"<<endl;
        getline(cin,pass2);
        if(pass!=pass2){
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
    cout<<"你注册的uid为:"<<uid<<endl;
    //cout<<"接下来将进入登录界面"<<endl<<endl;
    return;

}

int log_in(){
    string phone,pass,uid;
    int i=0;
    while(1){
        cout<<"请输入你的电话号码"<<endl;
        getline(cin,phone);
        cout<<"请输入你的密码"<<endl;
        getline(cin,pass);
        cout<<"请输入你的uid"<<endl;
        getline(cin,uid);
        log_uid=uid;

        Message msg(phone,uid,pass,LOGIN);
        
        socket_fd.mysend(msg.S_to_json());

        string recv=socket_fd.client_recv();
        if(recv=="该用户未注册"){
            cout<<"你还未注册，请先注册"<<endl;
            return 0;
        }else if(recv=="该用户已登录"){
            cout<<"你已经登录，请勿重复登录"<<endl;
        }else if(recv=="密码错误"){
            cout<<"密码错误"<<endl;
            i++;
            if(i==5) return 0;
            continue;
        }else if(recv=="ok"){
            cout<<"登录成功"<<endl;
            system("clear");
            thread   thread([uid=log_uid,noticefd=socket_fd.get_receive_fd()](){
                notice_recv_thread(uid,noticefd);
            });
            thread.detach();
            return 1;
        }


    }
}

void notice_recv_thread(string uid,int noticefd){
    StickyPacket noticesocket(noticefd);
    if(connect(noticefd,(sockaddr*)&client_addr,sizeof(client_addr))  <0){
        perror("connect failed!\n");
        return;
    }

    Message msg(uid,NOTICE);
    noticesocket.mysend(msg.S_to_json());

    while(1){
        string recv =noticesocket.client_recv();
        if(recv =="close"){
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
    if(recv=="no"){
        cout<<"你的uid与电话不匹配,无法找回密码"<<endl;
    }else if(recv=="yes"){
        string pass=socket_fd.client_recv();
        cout<< "你的密码是："<< pass<<endl;
    }

    return;

}