#include "server.hpp"
//注册
void sign_up(StickyPacket socket,Message &msg){
    string uid = redis.incr("user_uid_counter");
    //string uid =to_string(user_uid);
    
    //User user(msg.name,uid,msg.pass,msg.question,msg.para[0],msg.phone);
    
    //user_uid=1;
    cout<<"name:"<<msg.name<<endl;
    redis.hset(uid,"name",msg.name);
    //redis.hset(uid,"uid",msg.uid);
    redis.hset(uid,"pass",msg.pass);
    //redis.hset(uid,"question",msg.question);
    //redis.hset(uid,"answer",msg.para[0]);
    redis.hset(uid,"phone",msg.phone);

    /*redis.hset(uid, "聊天对象", "无");
    redis.hset(uid + "的未读消息", "通知类消息", "0");
    redis.hset(uid + "的未读消息", "好友消息", "0");
    redis.hset(uid + "的未读消息", "新的朋友", "0");
    redis.hset(uid + "的未读消息", "群聊申请", "0");
    redis.hset(uid + "的未读消息", "群聊消息", "0");*/
     
    cout<<"uid:"<<uid<<endl;
    
    redis.sadd("用户ID集合",uid);
    cout<<"uid:"<<uid<<endl;

    
    
    socket.mysend(uid);
    return;

}

void log_in(StickyPacket socket,Message &msg){
    //检查是否注册/已登录
    if(!redis.sismember("用户ID集合",msg.uid)){
        socket.mysend("该用户未注册");
    }
    //已登录
    else if(online_users.find(msg.uid)!=online_users.end()){
        socket.mysend("该用户已登录");
    }
    //登录
    else{
        string password=redis.Hget(msg.uid,"pass");
        //string telephone=redis.Hget(msg.uid,"phone");
        if(password!=msg.pass){
            cout<<"密码错误"<<endl;
            socket.mysend("密码错误");

        }
        else{
            cout<<"登录成功"<<endl;
            online_users.insert(msg.uid);
            //redis.hset("fd-uid表", to_string(socket.getfd()), msg.uid);
            //redis.hset(msg.uid, "聊天对象", "0");
            //string name = redis.Hget(msg.uid, "name");
            redis.hset(msg.uid, "聊天对象", "0");
            redis.hset(msg.uid + "的未读消息", "通知类消息", "0");
            redis.hset(msg.uid + "的未读消息", "好友消息", "0");
            redis.hset(msg.uid + "的未读消息", "新的朋友", "0");
            redis.hset(msg.uid + "的未读消息", "群聊申请", "0");
            redis.hset(msg.uid + "的未读消息", "群聊消息", "0");
            redis.hset(msg.uid, "消息fd", "-1");

            socket.mysend("ok");

        }
    }
}



void passfind(StickyPacket socket,Message &msg){
    string phone=redis.Hget(msg.uid,"phone");
    if(phone != msg.phone){
        socket.mysend("no");
        return;
    }else{
        socket.mysend("yes");
        string pass=redis.Hget(msg.uid,"pass");
        socket.mysend(pass);
        return;

    }
}


void client_quit(StickyPacket socket,Message &msg){
    online_users.erase(msg.uid);
    redis.hset(msg.uid,"消息fd","-1");
    socket.mysend("ok");
    int fd=socket.getfd();
    close(fd);
}
