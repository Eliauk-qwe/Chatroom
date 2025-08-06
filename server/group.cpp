#include "server.hpp"
void group_creat(StickyPacket socket,Message &msg){
    

    /*if(redis.sismember("群聊ID集合",msg.friend_or_group)){
        socket.mysend("name_have_exit");
        return;
    }*/


    /*string gid = redis.incr("group_uid_counter");
    redis.sadd("群聊ID集合",gid);
    redis.hset("群聊ID-NAME表",gid,msg.friend_or_group);*/
    if(msg.uid==msg.other){
        socket.mysend("my");
        return;
    }

    if(!redis.Hexists(msg.uid+"的好友列表",msg.other)){
        socket.mysend("no_friend");
        return;
    }

    if(redis.Hexists(msg.uid+"的好友列表",msg.other) && redis.Hexists(msg.other+"的好友列表",msg.uid)){
        socket.mysend("del");
        return;
    }

    if(!redis.sismember("用户ID集合",msg.other)){
        socket.mysend("no");
        return;
    }




    /*string gid;
    string notice="你已成功与uid为";
    int num=0;
    for(int i=0;i<msg.para.size();i++){
        if(!redis.Hexists(msg.uid+"的好友列表",msg.para[i])){
            continue;
        }
        num++;*/
    string  gid = redis.incr("group_uid_counter");
    redis.sadd("群聊ID集合",gid);
    redis.hset("群聊ID-NAME表",gid,msg.friend_or_group);

    for (int i = 0; i < msg.para.size(); i++)
    {
        redis.sadd(gid + "的群成员", msg.uid);
        redis.sadd(gid + "的群成员", msg.other);

        redis.hset(msg.uid + "的群聊列表", gid, msg.friend_or_group);
        redis.hset(msg.other + "的群聊列表", gid, msg.friend_or_group);
    }

    //redis.hset(msg.uid+"的群聊列表",gid,msg.friend_or_group);
    redis.hset(msg.uid+"创建的群聊",gid,msg.friend_or_group);
    //redis.hset(msg.uid+"的群聊消息",msg.friend_or_group,"0");

    //string name=redis.Hget(msg.uid,"name");
    redis.hset(gid,"群主",msg.uid);

    redis.sadd(gid+"的高权限者",msg.uid);

    //redis.sadd(gid+"的群成员",msg.uid);

    

    socket.mysend("ok");

    socket.mysend(gid);
    
}


void group_list(StickyPacket socket,Message &msg){
    
    if(!redis.Hlen(msg.uid+"的群聊列表")){
        socket.mysend("no");
        return;
    }

    vector<string> groupnamelist= redis.Hgetall(msg.uid+"的群聊列表");
    for(int i=0;i<groupnamelist.size();i=i+2){
        string notice=groupnamelist[i]+"  "+groupnamelist[i+1];
        string owner = redis.Hget(groupnamelist[i], "群主");
        if (msg.uid == owner)
        {
            socket.mysend(notice + "  群主");
        }
        else if (redis.sismember(groupnamelist[i] + "的管理员", msg.uid))
        {
            socket.mysend(notice + "  管理员");
        }
        else
        {
            socket.mysend(notice + "  普通成员");
        }
    }

    socket.mysend("over");
}

void group_add(StickyPacket socket,Message &msg){
    if(!redis.sismember("群聊ID集合",msg.friend_or_group)){
        socket.mysend("name_have_no_exit");
        return;
    }

    string name=redis.Hget("群聊ID-NAME表",msg.friend_or_group);

    if(redis.sismember(msg.friend_or_group+"的群成员",msg.uid)){
        socket.mysend("have_exit");
        return;
    }

    
    string group_owner_uid=redis.Hget(msg.friend_or_group,"群主");

    if(redis.Hexists(group_owner_uid+"的群聊申请",msg.uid)){
        socket.mysend("have_send");
        return;
    }
    //string group_owner_uid=redis.Hget(msg.friend_or_group,"群主");

    /*if(redis.Hexists(group_owner_uid+"的群聊申请",msg.uid)){
        socket.mysend("have_send");
        return;
    }*/

    //string group_owner_uid=redis.Hget(msg.friend_or_group,"群主");
    /*string num1=redis.Hget(group_owner_uid+"的未读消息","群聊申请");
    redis.hset(group_owner_uid+"的未读消息","群聊申请",to_string(stoi(num1)+1));
    redis.hset(group_owner_uid+"的群聊申请",msg.uid,"我想加入"+msg.friend_or_group+","+msg.other);
    
    if(online_users.find(group_owner_uid) != online_users.end()){
        string notice_fd=redis.Hget(group_owner_uid,"消息fd");
        StickyPacket notice_socket (stoi(notice_fd));
        string notice="收到来自"+msg.uid+"的加群申请";
        notice_socket.mysend(notice);

    }*/

    vector<string> group_managers=redis.Smembers(msg.friend_or_group+"的高权限者");
    for(const string & group_managers_uid :group_managers){
        if(group_managers_uid==msg.uid) continue;
        redis.hset(group_managers_uid+"的群聊申请",msg.uid,"我想加入群聊"+msg.friend_or_group+":"+name+",我的自我介绍是"+msg.other);

        string user_name=redis.Hget(msg.uid,"name");

        string num=redis.Hget(group_managers_uid+"的未读消息","通知类消息");
        redis.hset(group_managers_uid+"的未读消息","通知类消息",to_string(stoi(num)+1));
        redis.Rpush(group_managers_uid+"的通知类消息",msg.uid+":"+user_name+"申请加入群聊"+msg.friend_or_group+":"+name);


        if(online_users.find(group_managers_uid) != online_users.end()){
            string notice_fd=redis.Hget(group_managers_uid,"消息fd");
            StickyPacket notice_socket (stoi(notice_fd));
            string notice="收到来自"+msg.uid+"的加群申请";
            notice_socket.mysend(QING+notice+RESET);
        }

    }


    socket.mysend("ok");
    

}


void access_group(StickyPacket socket,Message &msg){
    if(!redis.sismember("群聊ID集合",msg.friend_or_group)){
        socket.mysend("no");
        return;

    }
    string name=redis.Hget("群聊ID-NAME表",msg.friend_or_group);

    if(!redis.sismember(msg.friend_or_group+"的群成员",msg.uid)){
        socket.mysend("quit");
        return;
    }
    
    socket.mysend(name);

    string owner=redis.Hget(msg.friend_or_group,"群主");
    if(msg.uid==owner){
        socket.mysend("1");
    }else if(redis.sismember(msg.friend_or_group+"的管理员",msg.uid)){
        socket.mysend("2");
    }
    else{
        socket.mysend("3");
    }
    
    return;
}
