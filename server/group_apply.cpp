#include "server.hpp"
void group_apply_agree(StickyPacket socket,Message &msg){
    if(redis.sismember(msg.other+"的群成员",msg.friend_or_group)){
        socket.mysend("have_exist");

        redis.Hdel(msg.uid+"的群聊申请",msg.friend_or_group);
        string num=redis.Hget(msg.uid+"的未读消息","群聊申请");
        redis.hset(msg.uid+"的未读消息","群聊申请",to_string(stoi(num)-1));
        return;
    }

    
    //对于uid
    redis.Hdel(msg.uid+"的群聊申请",msg.friend_or_group);
    string num=redis.Hget(msg.uid+"的未读消息","群聊申请");
    redis.hset(msg.uid+"的未读消息","群聊申请",to_string(stoi(num)-1));

    redis.sadd(msg.other+"的群成员",msg.friend_or_group);
    redis.sadd(msg.friend_or_group+"的群聊列表",msg.other);

    //对于person_uid
    redis.Rpush(msg.friend_or_group+"的通知类消息","你已被"+msg.uid+"拒绝加入群聊"+msg.other);

    if(online_users.find(msg.friend_or_group) != online_users.end()){
        string notice_fd=redis.Hget(msg.friend_or_group,"消息fd");
        StickyPacket notice_socket(stoi(notice_fd));
        string notice ="你已被"+msg.uid+"拒绝加入群聊"+msg.other;
        notice_socket.mysend(notice);;
    }

    socket.mysend("ok");

}


void group_apply_refuse(StickyPacket socket,Message &msg){
    if(redis.sismember(msg.other+"的群成员",msg.friend_or_group)){
        socket.mysend("have_exist");
        redis.Hdel(msg.uid+"的群聊申请",msg.friend_or_group);
        string num=redis.Hget(msg.uid+"的未读消息","群聊申请");
        redis.hset(msg.uid+"的未读消息","群聊申请",to_string(stoi(num)-1));

    }
    //对于uid
    redis.Hdel(msg.uid+"的群聊申请",msg.friend_or_group);
    string num=redis.Hget(msg.uid+"的未读消息","群聊申请");
    redis.hset(msg.uid+"的未读消息","群聊申请",to_string(stoi(num)-1));

    //redis.sadd(msg.other+"的群成员",msg.friend_or_group);
    //redis.sadd(msg.friend_or_group+"的群聊列表",msg.other);

    //对于person_uid
    //如果被多人拒绝呢？，通过表明被谁来区分
    redis.Rpush(msg.friend_or_group+"的通知类消息","你已被"+msg.uid+"拒绝加入群聊"+msg.other);

    if(online_users.find(msg.friend_or_group) != online_users.end()){
        string notice_fd=redis.Hget(msg.friend_or_group,"消息fd");
        StickyPacket notice_socket(stoi(notice_fd));
        string notice ="你已被"+msg.uid+"拒绝加入群聊"+msg.other;
        notice_socket.mysend(notice);;
    }

    socket.mysend("ok");

}



void check_group_apply(StickyPacket socket,Message &msg){
    if(!redis.Exists(msg.uid+"的群聊申请")){
        socket.mysend("no");
        return;
    }

    vector<string> all_group_apply =redis.Hgetall(msg.uid+"的群聊申请");
    for(const string &group_apply :all_group_apply){
        socket.mysend(group_apply);
    }
    socket.mysend("over");
}