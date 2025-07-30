#include "server.hpp"
/*void user_del(StickyPacket socket,Message &msg){
    vector<string> friendlist = redis.Hkeys(msg.uid,"的好友列表");
    for(const string &friendID : friendlist){
        redis.Hdel(friendID + "的好友列表",msg.uid);
    }
    
    redis.del(msg.uid + "的好友列表");
    redis.del(msg.uid + "的屏蔽列表");
    redis.del(msg.uid + "的未读消息");
    redis.del(msg.uid + "的通知消息");

    vector<string> grouplist = redis.Hkeys(msg.uid,"的群聊列表");
    for(const string &friendID : grouplist){
        redis.Hdel(friendID + "群成员的列表",msg.uid);
    }
    redis.del(msg.uid + "的群聊列表");
    redis.del(msg.uid);
    redis.Srem("user_id集合",msg.uid);
    /////////////////////?/???/？
    socket.mysend("success"); 
}*/


/*void unread_msg(StickyPacket socket,Message &msg){
    string notice;
    int con_num=stoi(redis.Hget(msg.uid + "的未读消息","通知类消息"));
    int app_num=stoi(redis.Hget(msg.uid + "的未读消息","好友申请"));
    int sum_num=con_num+app_num;

    if(sum_num==0){
        socket.mysend("fail");
        return;
    }
    else{
        notice="有"+to_string(sum_num)+"条未读消息\n";

        for(int i=0;i<con_num;i++){
            string key=msg.uid + "的通知消息";
            string each_notice=redis.Lindex(key,i);
            notice +="通知"+to_string(i+1)+":"+each_notice+"\n";
        }

        vector<string>  friend_app_list =redis.Hkeys(msg.uid,"收到的好友申请");
        for(int i=0;i<friend_app_list.size();i++){
            string friend_app =redis.Hget(msg.uid+"收到的好友申请",friend_app_list[i]);
            notice += "好友申请"+to_string(i+1)+":"+friend_app+"\n";
        }
        
    }

    socket.mysend(YELLOW + notice+ RESET);
    redis.del(msg.uid+"的通知消息");
    redis.hset(msg.uid + "的未读消息", "通知类消息", "0");
}*/



void unread_msg(StickyPacket socket,Message &msg){

    //通知类消息
    int num1=redis.Llen(msg.uid+"的通知类消息");
    string a="[1]通知类消息有"+to_string(num1)+"个,以下是你的通知消息";
    socket.mysend(a);
    vector<string> noticelist1=redis.Lrange(msg.uid+"的通知类消息");
    for(const string &notice:noticelist1){
        socket.mysend(notice);
    }
    redis.del(msg.uid+"的通知类消息");
    
    //新的朋友
    string num2=redis.Hget(msg.uid+"的未读消息","新的朋友");
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
    }

    socket.mysend("over");



}