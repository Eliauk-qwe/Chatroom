#include "server.hpp"
void friend_add(StickyPacket socket,Message &msg){
    //判断是不是friend在不在uid集合
    if(!redis.sismember("用户ID集合",msg.friend_or_group)){
        socket.mysend("该用户不存在");
        return;
    }

    //判断是不是friend在不在uid的好友列表
    if((redis.Hexists(msg.uid+"的好友列表",msg.friend_or_group)) &&(redis.Hexists(msg.friend_or_group+"的好友列表",msg.uid))){
        socket.mysend("friend_exit");
    }

    if(redis.Hexists(msg.uid+"的新的朋友",msg.friend_or_group)){
        socket.mysend("receive_friend_apply");
        return;
    }

    if(redis.Hexists(msg.friend_or_group+"的新的朋友",msg.uid)){
        socket.mysend("have_send");
        return;
    }

    if(msg.uid==msg.friend_or_group){
        socket.mysend("no");
        return;
    }
    
    string name =redis.Hget(msg.uid,"name");


    //string apply = msg.uid + ":"+ msg.other;
    redis.hset(msg.friend_or_group+"的新的朋友",msg.uid,msg.other);
    redis.Rpush(msg.friend_or_group+"的通知类消息","收到来自" + msg.uid +":"+name+ "的好友申请");


    /*string num=redis.Hget(msg.friend_or_group+"的未读消息","新的朋友");
    redis.hset(msg.friend_or_group+"的未读消息","新的朋友",to_string(stoi(num)+1));*/

    if(online_users.find(msg.friend_or_group) != online_users.end()){
        string friend_fd =redis.Hget(msg.friend_or_group,"消息fd");
        StickyPacket friend_socket(stoi(friend_fd));
        string notice = "收到来自" + msg.uid +":"+name+ "的好友申请";
        friend_socket.mysend(QING + notice + RESET);
        
    }

    socket.mysend("ok");


}

void friend_del(StickyPacket socket,Message &msg){
    if(msg.uid==msg.friend_or_group){
        socket.mysend("no");
        return;
    }
    if(!redis.Hexists(msg.uid+"的好友列表",msg.friend_or_group)){
        socket.mysend("friend_no_exist");
        return;
        
    }

    redis.Hdel(msg.uid + "的好友列表", msg.friend_or_group);
    // redis.Srem(msg.friend_or_group+"的好友列表",msg.uid);
    redis.del(msg.uid+"与"+msg.friend_or_group+"的聊天记录");
    //redis.del(msg.friend_or_group+"与"+msg.uid+"的聊天记录");
    socket.mysend("ok");
    return;
}

void friend_list(StickyPacket socket,Message &msg){
    if(!redis.Hlen(msg.uid+"的好友列表")){
        socket.mysend("no_exit");
        return;
    }

    vector<string> friendlist =redis.Hgetall(msg.uid+"的好友列表");
    /*for(const string &frienduid :friendlist){
        if(!redis.Hexists(msg.uid+"的屏蔽列表",frienduid)){
            if(online_users.find(frienduid)!=online_users.end()){
                socket.mysend(YELLOW+frienduid+RESET);
            }else{
                socket.mysend(frienduid);
            }
        }
    }*/

    for(size_t i=0;i<friendlist.size(); i+=2){
        const string &frienduid =friendlist[i];
        
            string notice=frienduid+" "+friendlist[i+1];
            if(online_users.find(frienduid)!=online_users.end()){
                socket.mysend(YELLOW+notice+"  在线"+RESET);
            }else{
                socket.mysend(PLUSWHITE+ notice+"  离线"+RESET);
            }
        
    }

    socket.mysend("over");
}


void friend_chat(StickyPacket socket,Message &msg){
    if(msg.uid==msg.friend_or_group){
        socket.mysend("my");
        return;
    }
    if(!redis.sismember("用户ID集合",msg.friend_or_group)){
        socket.mysend("no");
        return;
    }

    if(!redis.Exists(msg.uid+"与"+msg.friend_or_group+"的聊天记录")){
        socket.mysend("friend_no_exist");
        return;
    }

    if(!redis.Hexists(msg.uid+"的好友列表",msg.friend_or_group)){
        socket.mysend("no_exit");
        return;
    }

   if(!redis.Hlen(msg.uid+"的好友列表")){
        socket.mysend("0");
        return;
    }



    

    /*if(redis.Hexists(msg.friend_or_group+"的屏蔽列表",msg.uid)){
        socket.mysend("quit");
        return;
    }*/
    socket.mysend("start");

    vector<string> chat_what = redis.Lrange(msg.uid+"与"+msg.friend_or_group+"的聊天记录");
    printf("聊天的历史记录为：\n");
    for(const string &notice :chat_what){
        socket.mysend(notice);

    }
   
    redis.hset(msg.uid, "聊天对象", msg.friend_or_group);
    /*string num1=redis.Hget(msg.uid+"的好友消息",msg.friend_or_group);
    redis.hset(msg.uid+"的好友消息",msg.friend_or_group,"0");
    string num=redis.Hget(msg.uid+"的未读消息","好友消息");
    redis.hset(msg.uid+"的未读消息","好友消息",to_string(stoi(num)-stoi(num1)));*/

    socket.mysend("over");

    
}


void friend_quit_chat(StickyPacket socket,Message &msg){
    redis.hset(msg.uid, "聊天对象", "0");
    /*string uid=redis.Hget(msg.friend_or_group, "聊天对象");
    if(uid==msg.uid){
        redis.hset(msg.friend_or_group, "聊天对象", "0");
    }*/
    socket.mysend("ok");
    return;
}


void friend_chat_daily(StickyPacket socket,Message &msg){
    //获取两个客户端的实时socket
    string fd1 =redis.Hget(msg.uid,"消息fd");
    string fd2 =redis.Hget(msg.friend_or_group,"消息fd");
    StickyPacket fd1_socket(stoi(fd1));
    StickyPacket fd2_socket(stoi(fd2));

    //对于你被好友删除
    if((!redis.Hexists(msg.friend_or_group+"的好友列表",msg.uid)) &&(redis.Hexists(msg.uid+"的好友列表",msg.friend_or_group))){
        //socket.mysend("friend_del");
        string  notice="我:" RED "!" RESET +msg.other; 
        fd1_socket.mysend(notice);
        redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice);
        socket.mysend("del");
        return;
    }


    //对于你被好友屏蔽
    if(redis.Hexists(msg.friend_or_group+"的屏蔽列表",msg.uid)){
        string  notice="我:" RED "!" RESET +msg.other; 
        fd1_socket.mysend(notice);
        redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice);
        socket.mysend("quit");
        return; 
    }



    

    //正常聊天
    //把消息存入服务器
    string notice1 =PLUSWHITE "我："  RESET+ msg.other;
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice1);

    string name1=redis.Hget(msg.uid,"name");
    string notice2 =ZI+name1 + ":" RESET+msg.other;
    redis.Rpush(msg.friend_or_group+"与"+msg.uid+"的聊天记录",notice2);

    //对于你
    fd1_socket.mysend(notice1);

    //对于好友
    //发送给客户端2
    if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") == msg.uid)){
        fd2_socket.mysend(notice2);
    }
    else if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") != msg.uid)){
        fd2_socket.mysend(QING+msg.uid+":"+name1+"给你发了一条消息"+RESET);
        //总数量
        /*string num1=redis.Hget(msg.friend_or_group+"的未读消息","好友消息");
        redis.hset(msg.friend_or_group+"的未读消息","好友消息",to_string(stoi(num1)+1));
        //单个好友数量
        string num2=redis.Hget(msg.uid+"的好友消息",msg.friend_or_group);
        redis.hset(msg.uid+"的好友消息",msg.friend_or_group,to_string(stoi(num1)+1));*/
        
    }
    else if(online_users.find(msg.friend_or_group)==online_users.end()){
        //总数量
        /*string num1=redis.Hget(msg.friend_or_group+"的未读消息","好友消息");
        redis.hset(msg.friend_or_group+"的未读消息","好友消息",to_string(stoi(num1)+1));
        //单个好友数量
        string num2=redis.Hget(msg.uid+"的好友消息",msg.friend_or_group);
        redis.hset(msg.uid+"的好友消息",msg.friend_or_group,to_string(stoi(num1)+1));*/
        
    }
    socket.mysend("ok");
    return;


}
































