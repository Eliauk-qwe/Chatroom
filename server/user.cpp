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
    printf("11111111111\n");
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

    
    //新的朋友
    /*string num2=redis.Hget(msg.uid+"的未读消息","新的朋友");
    string b="[2]新的朋友消息有"+num2+"个";
    socket.mysend(b);

    //好友消息
    string num3=redis.Hget(msg.uid+"的未读消息","好友消息");
    string c="[3]好友消息共有"+num3+"个,以下是你的分别未读的好友消息";
    socket.mysend(c);
    vector<string> noticelist2=redis.Hgetall(msg.uid+"的好友消息");
    for(int i=0;i<noticelist2.size();i+=2){
        string name =redis.Hget(noticelist2[i],"name");
        const string &notice=noticelist2[i]+":"+name+"    "+noticelist2[i+1];
        socket.mysend(notice);
    }

    //群聊申请
    string num4=redis.Hget(msg.uid+"的未读消息","群聊申请");
    string d="[4]群聊申请有"+num4+"个";
    socket.mysend(d);

    //群聊消息
    string num5=redis.Hget(msg.uid+"的未读消息","群聊消息");
    string e="[5]群聊消息共有"+num5+"个,以下是你的分别未读的群聊消息";
    socket.mysend(e);
    vector<string> noticelist3=redis.Hgetall(msg.uid+"的群聊消息");
    for(int i=0;i<noticelist3.size();i+=2){
        const string &notice=noticelist3[i]+"   "+noticelist3[i+1];
        socket.mysend(notice);
    }*/




}