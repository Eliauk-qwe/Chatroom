#include "server.hpp"
void friend_quit(StickyPacket socket,Message &msg){
    if(!redis.sismember(msg.uid+"的好友列表",msg.friend_or_group)){
        socket.mysend("friend_no_exist");
        return;
    }

    if(redis.sadd(msg.uid+"的屏蔽列表",msg.friend_or_group)){
        socket.mysend("success");
    }else{
        socket.mysend("fail");
    }
    return;
}

void friend_quit_list(StickyPacket socket,Message &msg){
    if(!redis.Exists(msg.uid+"的屏蔽列表")){
        socket.mysend("no_have_quit_list");
        return;
    }

    vector<string> quitlist=redis.Smembers(msg.uid+"的屏蔽列表");
    for(const string &quitfriendID: quitlist){
        if(redis.sismember(msg.uid+"的屏蔽列表",quitfriendID)){
            socket.mysend(quitfriendID);
        }
    }

    socket.mysend("end");
}


void friend_back(StickyPacket socket,Message &msg){
    if(!redis.sismember(msg.uid+"的屏蔽列表",msg.friend_or_group)){
        socket.mysend("no_quit_freind");
        return;
    }
    else if(!redis.sismember(msg.uid+"的好友列表",msg.friend_or_group)){
        socket.mysend("no_add");
        return;
    }
    else if(redis.Srem(msg.uid+"的屏蔽列表",msg.friend_or_group)){
        socket.mysend("ok");
        return;
    }

}

