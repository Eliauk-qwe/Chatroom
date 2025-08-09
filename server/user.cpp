#include "server.hpp"
void user_quit(StickyPacket socket,Message &msg){
    vector<string> friendlist = redis.Hgetall(msg.uid+"的好友列表");
    for(int i=0;i<friendlist.size();i=i+2){
        redis.del(msg.uid+"与"+friendlist[i]+"的聊天记录");
    }
    redis.del(msg.uid + "的未读消息");
    redis.del(msg.uid + "的好友列表");
    redis.del(msg.uid + "的屏蔽列表");
    redis.del(msg.uid+"创建的群聊");
    redis.del(msg.uid + "的通知类消息");
    redis.del(msg.uid + "的新的朋友");
    redis.del(msg.uid + "的好友消息");
    redis.del(msg.uid + "的群聊申请");
    redis.del(msg.uid + "的群聊消息");
    redis.del(msg.uid + "的群聊列表");
    redis.del(msg.uid + "创建的群聊");
    


    vector<string> grouplist = redis.Smembers(msg.uid+"的群聊列表");
    for(const string &groupname : grouplist){
        redis.del(msg.uid + "与"+groupname+"的聊天记录");
    }
    redis.del(msg.uid + "的群聊列表");
    redis.del(msg.uid);
    redis.Srem("用户ID集合",msg.uid);
    if (online_users.find(msg.uid) != online_users.end())
    {
        online_users.erase(msg.uid);
    }
    socket.mysend("ok"); 
}




void notice(StickyPacket socket,Message &msg){
    
    //通知类消息
    int num1=redis.Llen(msg.uid+"的通知类消息");
    //cout<<num1<<endl;
    /*if(num1==0){
        socket.mysend("0");
    }*/
    string a="通知类消息有"+to_string(num1)+"个,以下是你的通知消息";
    socket.mysend(a);
    vector<string> noticelist1=redis.Lrange(msg.uid+"的通知类消息");
    for(const string &notice:noticelist1){
        socket.mysend(PLUSWHITE+notice+RESET);
    }
    redis.del(msg.uid+"的通知类消息");
    socket.mysend("over");

    
   


}
