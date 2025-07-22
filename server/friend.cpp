#include "server.hpp"
void friend_add(StickyPacket socket,Message &msg){
    //判断是不是friend在不在uid集合
    if(!redis.sismember("用户ID集合",msg.friend_or_group)){
        socket.mysend("该用户不存在");
        return;
    }

    //判断是不是friend在不在uid的好友列表
    string name=redis.Hget(msg.friend_or_group,"name");


    bool flag=redis.Hexists(msg.uid+"的好友列表",name);
    if(flag==true)  socket.mysend("friend_exit");
    /*if(num!=0){
        vector<string> friendlist =redis.Hgetall(msg.uid,"的好友列表");
        for(const string &friendID : friendlist){
            if(friendID==msg.friend_or_group){
                socket.mysend("friend_exit");
                return;
            }
        }
    }*/

    if(redis.sismember(msg.uid+"的新的朋友",msg.friend_or_group)){
        socket.mysend("receive_friend_apply");
        return;
    }

    if(redis.sismember(msg.friend_or_group+"的新的朋友",msg.uid)){
        socket.mysend("have_send");
        return;
    }
    

    //string apply = msg.uid + ":"+ msg.other;
    redis.hset(msg.friend_or_group+"的好友申请",msg.uid,msg.other);

    string count=redis.Hget(msg.friend_or_group+"的未读消息","新的朋友");
    redis.hset(msg.friend_or_group+"的未读消息","新的朋友",to_string(stoi(count)+1));

    if(online_users.find(msg.friend_or_group) != online_users.end()){
        string friend_fd =redis.Hget(msg.friend_or_group,"消息fd");
        StickyPacket friend_socket(stoi(friend_fd));
        string notice = "收到来自" + msg.uid + "的好友申请";
        friend_socket.mysend(RED + notice + RESET);
        
    }

    socket.mysend("ok");


}

void friend_del(StickyPacket socket,Message &msg){
    if(!redis.sismember(msg.uid+"的好友列表",msg.friend_or_group)){
        socket.mysend("friend_no_exist");
        return;
        
    }else if(redis.Srem(msg.uid+"的好友列表",msg.friend_or_group)){
        redis.Srem(msg.friend_or_group+"的好友列表",msg.uid);
        socket.mysend("ok");
        return;
    }
}

void friend_list(StickyPacket socket,Message &msg){
    if(!redis.Exists(msg.uid+"的好友列表")){
        socket.mysend("no_exit");
        return;
    }

    vector<string> friendlist =redis.Smembers(msg.uid+"的好友列表");
    for(const string &friendname :friendlist){
        if(!redis.sismember(msg.uid+"的屏蔽列表",friendname)){
            if(online_users.find(friendname)!=online_users.end()){
                socket.mysend(YELLOW+friendname+RESET);
            }else{
                socket.mysend(friendname);
            }
        }
    }

    socket.mysend("over");
}


void friend_chat(StickyPacket socket,Message &msg){
    if(!redis.sismember(msg.uid+"的好友列表",msg.friend_or_group)){
        socket.mysend("friend_no_exist");
        return;
    }
    socket.mysend("start");

    vector<string> chat_what = redis.Lrange(msg.uid+"与"+msg.friend_or_group+"的聊天记录");
    printf("聊天的历史记录为：\n");
    for(const string &notice :chat_what){
        socket.mysend(notice);

    }
///////////////////////////////////////////////////?/？？？？
    redis.hset(msg.uid, "聊天对象", msg.friend_or_group);

    socket.mysend("end");

}

void friend_chat_daily(StickyPacket socket,Message &msg){
    //把消息存入服务器
    string notice1 = "我：" + msg.para[0];
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice1);

    string notice2 =msg.uid + ":"+msg.para[0];
    redis.Rpush(msg.friend_or_group+"与"+msg.uid+"的聊天记录",notice2);

    //获取两个客户端的实时socket
    string fd1 =redis.Hget(msg.uid,"实时socket");
    string fd2 =redis.Hget(msg.friend_or_group,"实时socket");

    StickyPacket fd1_socket(stoi(fd1));
    StickyPacket fd2_socket(stoi(fd2));

    //回显给客户端1
    fd1_socket.mysend(notice1);
    

    //客户端2屏蔽了客户端1
    if(redis.sismember(msg.friend_or_group+"的屏蔽列表",msg.uid)){
        fd1_socket.mysend("你已被对方屏蔽");
        socket.mysend("have_been_quit");
        return;
    }

    //发送给客户端2
    if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") == msg.uid)){
        fd2_socket.mysend(notice2);
    }
    else if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") != msg.uid)){
        fd2_socket.mysend(msg.uid+"给你发了一条消息");
    }
    else if(online_users.find(msg.friend_or_group)==online_users.end()){
        string num=redis.Hget(msg.friend_or_group+"的未读消息","通知类消息");
        redis.hset(msg.friend_or_group+"的未读消息","通知类消息",to_string(stoi(num)));
        redis.Rpush(msg.friend_or_group+"的通知消息",msg.uid + "给你发来了一条消息");
    }


    socket.mysend("over");
    return;

}


void friend_quit_chat(StickyPacket socket,Message &msg){
    redis.hset(msg.uid, "聊天对象", "0");
    /////////////////////////////////////???//?/
    redis.hset(msg.friend_or_group, "聊天对象", "0");
    socket.mysend("success");
    return;
}







