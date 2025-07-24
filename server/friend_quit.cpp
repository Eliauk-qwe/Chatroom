#include "server.hpp"
void friend_quit(StickyPacket socket,Message &msg){
    if(!redis.Hexists(msg.uid+"的好友列表",msg.friend_or_group)){
        socket.mysend("friend_no_exist");
        return;
    }

    if(redis.Hexists(msg.uid+"的屏蔽列表",msg.friend_or_group)){
        socket.mysend("friend_have_exist");
        return;
    }

    string name=redis.Hget(msg.friend_or_group,"name");

    if(redis.hset(msg.uid+"的屏蔽列表",msg.friend_or_group,name)){
        socket.mysend("ok");
        return;
    }
}

void friend_quit_list(StickyPacket socket,Message &msg){
    if(!redis.Hlen(msg.uid+"的屏蔽列表")){
        socket.mysend("no_have_quit_list");
        return;
    }

    vector<string> quitlist=redis.Hgetall(msg.uid+"的屏蔽列表");
    for(size_t i=0 ;i<quitlist.size();i+=2){
        const string &quitfriend =quitlist[i];
        string notice =quitlist[i]+"  "+quitlist[i+1];
        socket.mysend(notice); 
    }

    socket.mysend("over");
}


void friend_back(StickyPacket socket,Message &msg){
    if(!redis.Hexists(msg.uid+"的屏蔽列表",msg.friend_or_group)){
        socket.mysend("no_quit_freind");
        return;
    }
    else if(!redis.Hexists(msg.uid+"的好友列表",msg.friend_or_group)){
        socket.mysend("no_add");
        return;
    }
    else if(redis.Hdel(msg.uid+"的屏蔽列表",msg.friend_or_group)){
        socket.mysend("ok");
        return;
    }

}

