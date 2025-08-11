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
        return;
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
    //string fd1 =redis.Hget(msg.uid,"消息fd");
    string fd2 =redis.Hget(msg.friend_or_group,"消息fd");
    //StickyPacket fd1_socket(stoi(fd1));
    StickyPacket fd2_socket(stoi(fd2));

    //对于你被好友删除
    if((!redis.Hexists(msg.friend_or_group+"的好友列表",msg.uid)) &&(redis.Hexists(msg.uid+"的好友列表",msg.friend_or_group))){
        //socket.mysend("friend_del");
        string  notice="我:" RED "!" RESET +msg.other; 
        //fd1_socket.mysend(notice);
        redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",notice);
        socket.mysend("del");
        return;
    }


    //对于你被好友屏蔽
    if(redis.Hexists(msg.friend_or_group+"的屏蔽列表",msg.uid)){
        string  notice="我:" RED "!" RESET +msg.other; 
        //fd1_socket.mysend(notice);
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
    //fd1_socket.mysend(notice1);

    //对于好友
    //发送给客户端2
    if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") == msg.uid)){
        fd2_socket.mysend(notice2);
    }
    else if((online_users.find(msg.friend_or_group)!=online_users.end())  &&  (redis.Hget(msg.friend_or_group,"聊天对象") != msg.uid)){
        fd2_socket.mysend(QING+msg.uid+":"+name1+"给你发了一条消息"+RESET);
        
    }
    
    //close(stoi(fd2));
    socket.mysend("ok");
    return;


}


// void friend_chat_daily(StickyPacket socket, Message &msg) {
//     // 获取两个客户端的实时socket（带锁访问在线用户）
//     string fd1 = redis.Hget(msg.uid, "消息fd");
//     string fd2 = redis.Hget(msg.friend_or_group, "消息fd");
//     StickyPacket fd1_socket(stoi(fd1));
//     StickyPacket fd2_socket(stoi(fd2));

//     // 检查是否被对方删除
//     if ((!redis.Hexists(msg.friend_or_group + "的好友列表", msg.uid)) && 
//         (redis.Hexists(msg.uid + "的好友列表", msg.friend_or_group))) {
//         string notice = "我:" RED "!" RESET + msg.other; 
//         fd1_socket.mysend(notice);
//         redis.Rpush(msg.uid + "与" + msg.friend_or_group + "的聊天记录", notice);
//         socket.mysend("del");
//         return;
//     }

//     // 检查是否被对方屏蔽
//     if (redis.Hexists(msg.friend_or_group + "的屏蔽列表", msg.uid)) {
//         string notice = "我:" RED "!" RESET + msg.other; 
//         fd1_socket.mysend(notice);
//         redis.Rpush(msg.uid + "与" + msg.friend_or_group + "的聊天记录", notice);
//         socket.mysend("quit");
//         return; 
//     }

//     // 存储消息（同步操作，确保消息不丢失）
//     string name1 = redis.Hget(msg.uid, "name");
//     string notice1 = PLUSWHITE "我：" RESET + msg.other;
//     string notice2 = ZI + name1 + ":" RESET + msg.other;
//     redis.Rpush(msg.uid + "与" + msg.friend_or_group + "的聊天记录", notice1);
//     redis.Rpush(msg.friend_or_group + "与" + msg.uid + "的聊天记录", notice2);

//     // 给自己发送消息（同步操作）
//     fd1_socket.mysend(notice1);

//     // 构造给好友发送消息的任务（异步处理）
//     auto send_task = [=]() {
//         // 访问在线用户集合时加锁
//         std::lock_guard<std::mutex> lock(online_users_mutex);
//         bool is_online = (online_use//         socket.mysend("del");
//rs.find(msg.friend_or_group) != online_users.end());
        
//         if (is_online) {
//             string chat_target = redis.Hget(msg.friend_or_group, "聊天对象");
//             if (chat_target == msg.uid) {
//                 // 对方正在与当前用户聊天，直接发送消息
//                 fd2_socket.mysend(notice2);
//             } else {
//                 // 对方在其他聊天窗口，发送通知
//                 string notify = QING + msg.uid + ":" + name1 + "给你发了一条消息" + RESET;
//                 fd2_socket.mysend(notify);
//             }
//         } else {
//             // 对方离线，更新未读计数（这里简化处理，实际应加锁操作Redis）
//             string num = redis.Hget(msg.friend_or_group + "的未读消息", "好友消息");
//             redis.hset(msg.friend_or_group + "的未读消息", "好友消息", to_string(stoi(num) + 1));
//         }
//     };

//     // 任务入队，异步执行
//     msg_queue.push(send_task);

//     // 立即返回，不等待异步任务完成
//     socket.mysend("ok");
// }


// void friend_chat_daily(StickyPacket socket, Message &msg) {

//     // 对于你被好友删除
//     if ((!redis.Hexists(msg.friend_or_group + "的好友列表", msg.uid)) && (redis.Hexists(msg.uid + "的好友列表", msg.friend_or_group)))
//     {
//         string fd1 = redis.Hget(msg.uid, "消息fd");
//         string fd2 = redis.Hget(msg.friend_or_group, "消息fd");
//         StickyPacket fd1_socket(stoi(fd1));
//         StickyPacket fd2_socket(stoi(fd2));
//         // socket.mysend("friend_del");
//         string notice = "我:" RED "!" RESET + msg.other;
//         fd1_socket.mysend(notice);
//         redis.Rpush(msg.uid + "与" + msg.friend_or_group + "的聊天记录", notice);
//         socket.mysend("del");
//         return;
//     }

//     // 对于你被好友屏蔽
//     if (redis.Hexists(msg.friend_or_group + "的屏蔽列表", msg.uid))
//     {
//         string fd1 = redis.Hget(msg.uid, "消息fd");
//         string fd2 = redis.Hget(msg.friend_or_group, "消息fd");
//         StickyPacket fd1_socket(stoi(fd1));
//         StickyPacket fd2_socket(stoi(fd2));
//         string notice = "我:" RED "!" RESET + msg.other;
//         fd1_socket.mysend(notice);
//         redis.Rpush(msg.uid + "与" + msg.friend_or_group + "的聊天记录", notice);
//         socket.mysend("quit");
//         return;
//     }
//     string queue_message=msg.uid+"|"+msg.friend_or_group+"|"+msg.other;
//     redis.enqueue_message("friend_message_queue",queue_message);
//     socket.mysend("ok");
// }


/*void is_friend_chat_daily(StickyPacket socket,Message &msg,Redis redis){
    //获取两个客户端的实时socket
    string fd1 =redis.Hget(msg.uid,"消息fd");
    string fd2 =redis.Hget(msg.friend_or_group,"消息fd");
    StickyPacket fd1_socket(stoi(fd1));
    StickyPacket fd2_socket(stoi(fd2));

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
        
    }
    
    socket.mysend("ok");
    return;


}*/
































