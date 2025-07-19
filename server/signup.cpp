#include "server.hpp"
//注册
void sign_up(StickyPacket socket,Message &msg){
    pthread_mutex_t lock;
    pthread_mutex_init(&lock,NULL);
    pthread_mutex_lock(&lock);
    //user_id集合中个数+1
    user_uid+=redis.scard("用户ID集合");
    pthread_mutex_unlock(&lock);
    string uid =to_string(user_uid);
    User user(msg.name,uid,msg.pass,msg.question,msg.para[0],msg.phone);
    
    //user_uid=1;
    redis.hset(uid,"name",msg.name);
    //redis.hset(uid,"uid",msg.uid);
    redis.hset(uid,"pass",msg.pass);
    //redis.hset(uid,"question",msg.question);
    //redis.hset(uid,"answer",msg.para[0]);
    redis.hset(uid,"phone",msg.phone);

    redis.hset(uid, "聊天对象", "无");
    redis.hset(uid + "的未读消息", "通知类消息", "0");
    redis.hset(uid + "的未读消息", "好友申请", "0");
    redis.hset(uid + "的未读消息", "群聊消息", "0");

    redis.sadd("用户ID集合",uid);

    
    socket.mysend(uid);
    return;

}

void log_in(StickyPacket socket,Message &msg){
    //检查是否注册/已登录
    int flag=redis.sismember("用户ID集合",msg.uid);
    //未注册
    if(flag==0){
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
            redis.hset("fd-uid表", to_string(socket.getfd()), msg.uid);
            redis.hset(msg.uid, "聊天对象", "0");
            redis.hset(msg.uid, "消息fd", "-1");
            //string name = redis.Hget(msg.uid, "name");
            socket.mysend("ok");

        }
    }
}


void user_del(StickyPacket socket,Message &msg){
    vector<string> friendlist = redis.Hkeys(msg.uid,"的好友列表");
    for(const string &friendID : friendlist){
        redis.Hdel(friendID + "的好友列表",msg.uid);
    }
    
    redis.del(msg.uid + "的好友列表");
    redis.del(msg.uid + "的屏蔽列表");
    redis.del(msg.uid + "的未读消息");
    redis.del(msg.uid + "的通知消息");

    vector<string> grouplist = redis.Hkeys(msg.uid,"的群聊列表");
    for(const string &friendID : grouplist){
        redis.Hdel(friendID + "群成员的列表",msg.uid);
    }
    redis.del(msg.uid + "的群聊列表");
    redis.del(msg.uid);
    redis.Srem("user_id集合",msg.uid);
    /////////////////////?/???/？
    socket.mysend("success"); 
}

void unread_msg(StickyPacket socket,Message &msg){
    string notice;
    int con_num=stoi(redis.Hget(msg.uid + "的未读消息","通知类消息"));
    int app_num=stoi(redis.Hget(msg.uid + "的未读消息","好友申请"));
    int sum_num=con_num+app_num;

    if(sum_num==0){
        socket.mysend("fail");
        return;
    }
    else{
        notice="有"+to_string(sum_num)+"条未读消息\n";

        for(int i=0;i<con_num;i++){
            string key=msg.uid + "的通知消息";
            string each_notice=redis.Lindex(key,i);
            notice +="通知"+to_string(i+1)+":"+each_notice+"\n";
        }

        vector<string>  friend_app_list =redis.Hkeys(msg.uid,"收到的好友申请");
        for(int i=0;i<friend_app_list.size();i++){
            string friend_app =redis.Hget(msg.uid+"收到的好友申请",friend_app_list[i]);
            notice += "好友申请"+to_string(i+1)+":"+friend_app+"\n";
        }
        
    }

    socket.mysend(YELLOW + notice+ RESET);
    redis.del(msg.uid+"的通知消息");
    redis.hset(msg.uid + "的未读消息", "通知类消息", "0");
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
