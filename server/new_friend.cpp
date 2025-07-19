#include "server.hpp"

void friend_apply_agree(StickyPacket socket,Message &msg){
    

     if(redis.Hdel(msg.uid+"的好友申请",msg.friend_or_group)){

        //redis.Hdel(msg.uid+"的好友申请",msg.friend_or_group);

        string num1=redis.Hget(msg.uid+"的未读消息","新的朋友");
        redis.hset(msg.uid+"的未读消息","新的朋友",(to_string(stoi(num1)-1)));

        string name1=redis.Hget(msg.uid,"name");
        string name2=redis.Hget(msg.friend_or_group,"name");

        redis.sadd(msg.uid+"的好友列表",name2);
        redis.sadd(msg.friend_or_group+"的好友列表",name1);

        redis.Rpush(name1+"与"+name2+"的聊天记录","---------");
        redis.Rpush(name2+"与"+name1+"的聊天记录","---------");
        redis.Rpush(name2+"与"+name1+"的聊天记录",name1+"通过了你的好友申请");

        string num2=redis.Hget(msg.friend_or_group+"的未读消息","通知类消息");
        redis.hset(msg.friend_or_group+"的未读消息","通知类消息",(to_string(stoi(num2)+1)));


        redis.Rpush(msg.friend_or_group+"的通知消息",msg.uid+"通过了你的好友申请");

        if(online_users.find(msg.friend_or_group) != online_users.end()){
            string friend_fd = redis.Hget(msg.friend_or_group,"消息fd");
            StickyPacket friendsocket (stoi(friend_fd));
            friendsocket.mysend(name1+"通过了你的好友申请");
        }

        socket.mysend("ok");
        return;


    }
    
}

void friend_apply_refuse(StickyPacket socket,Message &msg){
    

     if(redis.Hdel(msg.uid+"的好友申请",msg.friend_or_group)){
        string num1=redis.Hget(msg.uid+"的未读消息","新的朋友");
        redis.hset(msg.uid+"的未读消息","新的朋友",(to_string(stoi(num1)-1)));

        
        
        //redis.hset(msg.uid+"的好友列表",msg.friend_or_group,"ok");
        //redis.hset(msg.friend_or_group+"的好友列表",msg.uid,"ok");

        //redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录","---------");
        //redis.Rpush(msg.friend_or_group+"与"+msg.uid+"的聊天记录","---------");
        //redis.Rpush(msg.friend_or_group+"与"+msg.uid+"的聊天记录",msg.uid+"通过了你的好友申请");
        string name1=redis.Hget(msg.uid,"name");
        string name2=redis.Hget(msg.friend_or_group,"name");

        string num2=redis.Hget(msg.friend_or_group+"的未读消息","通知类消息");
        redis.hset(msg.friend_or_group+"的未读消息","通知类消息",(to_string(stoi(num2)+1)));

        string notice=name1+"拒绝了你的好友申请";
        redis.sadd(name2+"的通知类消息",notice);
        redis.Rpush(msg.friend_or_group+"的通知消息",msg.uid+"拒绝了你的好友申请");

        if(online_users.find(msg.friend_or_group) != online_users.end()){
            string friend_fd = redis.Hget(msg.friend_or_group,"消息fd");
            StickyPacket friendsocket (stoi(friend_fd));
            friendsocket.mysend(msg.uid+"拒绝了你的好友申请");
        }

        socket.mysend("success");
        return;


    }
    
}


void check_friend_apply(StickyPacket socket,Message &msg){
    if (!redis.Exists(msg.uid + "的好友申请"))
    {
        socket.mysend("no");
        return;
    }
    vector<string> friendapplylist = redis.Hgetall(msg.uid+"的好友申请");
    for (const string &friendapply : friendapplylist)
    {
        socket.mysend(friendapply);
        
    }
    socket.mysend("over");
}