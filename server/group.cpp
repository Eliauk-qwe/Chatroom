#include "server.hpp"
void group_creat(StickyPacket socket,Message &msg){
    if(redis.sismember("群聊名字集合",msg.friend_or_group)){
        socket.mysend("name_have_exit");
        return;
    }


    for(int i=0;i<msg.para.size();i++){
        redis.sadd(msg.friend_or_group+"的群成员",msg.para[i]);
        redis.sadd(msg.para[i]+"的群聊列表",msg.friend_or_group);
        redis.hset(msg.para[i]+"的群聊消息",msg.friend_or_group,"0");

    } 

    redis.sadd(msg.uid+"的群聊列表",msg.friend_or_group);
    redis.sadd(msg.uid+"创建的群聊",msg.friend_or_group);
    redis.hset(msg.uid+"的群聊消息",msg.friend_or_group,"0");

    //string name=redis.Hget(msg.uid,"name");
    redis.hset(msg.friend_or_group,"群主",msg.uid);

    redis.sadd(msg.friend_or_group+"的高权限者",msg.uid);

    redis.sadd(msg.friend_or_group+"的群成员",msg.uid);

    redis.sadd("群聊名字集合",msg.friend_or_group);
    socket.mysend("ok");
}


void group_list(StickyPacket socket,Message &msg){
    
    if(!redis.scard(msg.uid+"的群聊列表")){
        socket.mysend("no");
        return;
    }

    vector<string> groupnamelist= redis.Smembers(msg.uid+"的群聊列表");
    for(const string &groupname:groupnamelist){
        socket.mysend(groupname);
    }
    socket.mysend("over");
}

void group_add(StickyPacket socket,Message &msg){
    if(!redis.sismember("群聊名字集合",msg.friend_or_group)){
        socket.mysend("name_have_no_exit");
        return;
    }

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
        redis.hset(group_managers_uid+"的群聊申请",msg.uid,"我想加入"+msg.friend_or_group+","+msg.other);

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
    if(!redis.sismember("群聊名字集合",msg.friend_or_group)){
        socket.mysend("no");
        return;

    }

    if(!redis.sismember(msg.friend_or_group+"的群成员",msg.uid)){
        socket.mysend("quit");
        return;
    }
    redis.sadd(msg.friend_or_group+"的在线用户",msg.uid);
    
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
