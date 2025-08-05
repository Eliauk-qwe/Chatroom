#include "server.hpp"
void group_apply_agree(StickyPacket socket,Message &msg){
    if(!redis.sismember("群聊ID集合",msg.other)){
        socket.mysend("group_no_exit");
        return;
    }

    string group_name=redis.Hget("群聊ID-NAME表",msg.other);

    if(redis.sismember(msg.other+"的群成员",msg.friend_or_group)){
        socket.mysend("have_exist");
        redis.Hdel(msg.uid+"的群聊申请",msg.friend_or_group);
        return;
    }

    if(!redis.Hexists(msg.uid+"的群聊申请",msg.friend_or_group)){
        socket.mysend("nosend");
        return;
    }

    
    //对于uid
    redis.Hdel(msg.uid+"的群聊申请",msg.friend_or_group);

    //对于person_uid
    redis.sadd(msg.other+"的群成员",msg.friend_or_group);
    redis.hset(msg.friend_or_group+"的群聊列表",msg.other,group_name);
    //redis.hset(msg.friend_or_group+"的群聊消息",msg.other,"0");
    redis.Rpush(msg.friend_or_group+"的通知类消息","你已被同意加入群聊"+msg.other+":"+group_name);
    string num=redis.Hget(msg.friend_or_group+"的未读消息","通知类消息");
    redis.hset(msg.friend_or_group+"的未读消息","通知类消息",to_string(stoi(num)+1));

    if(online_users.find(msg.friend_or_group) != online_users.end()){
        string notice_fd=redis.Hget(msg.friend_or_group,"消息fd");
        StickyPacket notice_socket(stoi(notice_fd));
        string notice ="你已被同意加入群聊"+msg.other+":"+group_name;
        notice_socket.mysend(QING+notice+RESET);;
    }

    socket.mysend("ok");

}


void group_apply_refuse(StickyPacket socket,Message &msg){
    if(!redis.sismember("群聊ID集合",msg.other)){
        socket.mysend("group_no_exit");
        return;
    }

    string group_name=redis.Hget("群聊ID-NAME表",msg.other);


    if(redis.sismember(msg.other+"的群成员",msg.friend_or_group)){
        socket.mysend("have_exist");
        redis.Hdel(msg.uid+"的群聊申请",msg.friend_or_group);
        return;
    }

    if(!redis.Hexists(msg.uid+"的群聊申请",msg.friend_or_group)){
        socket.mysend("nosend");
        return;
    }

    //对于uid
    redis.Hdel(msg.uid+"的群聊申请",msg.friend_or_group);

    //redis.sadd(msg.other+"的群成员",msg.friend_or_group);
    //redis.sadd(msg.friend_or_group+"的群聊列表",msg.other);

    //对于person_uid
    //如果被多人拒绝呢？，通过表明被谁来区分
    redis.Rpush(msg.friend_or_group+"的通知类消息","你已被"+msg.uid+"拒绝加入群聊"+msg.other+":"+group_name);
    string num=redis.Hget(msg.friend_or_group+"的未读消息","通知类消息");
    redis.hset(msg.friend_or_group+"的未读消息","通知类消息",to_string(stoi(num)+1));


    if(online_users.find(msg.friend_or_group) != online_users.end()){
        string notice_fd=redis.Hget(msg.friend_or_group,"消息fd");
        StickyPacket notice_socket(stoi(notice_fd));
        string notice ="你已被"+msg.uid+"拒绝加入群聊"+msg.other+":"+group_name;
        notice_socket.mysend(QING+notice+RESET);
    }

    socket.mysend("ok");

}



void check_group_apply(StickyPacket socket,Message &msg){
    if(!redis.Hlen(msg.uid+"的群聊申请")){
        socket.mysend("no");
        return;
    }

    vector<string> all_group_apply =redis.Hgetall(msg.uid+"的群聊申请");
    for(int i=0;i<all_group_apply.size();i=i+2){
        string notice =all_group_apply[i]+"  "+all_group_apply[i+1];
        socket.mysend(PLUSWHITE+ notice+RESET);
    }

    //redis.hset(msg.uid+"的未读消息","群聊申请","0");


    socket.mysend("over");
}