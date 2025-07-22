#include "server.hpp"
void check_group_members(StickyPacket socket,Message &msg){
    if(!redis.sismember("群聊名字集合",msg.friend_or_group)){
        socket.mysend("no_group_name");
        return;
    }

    vector<string> groupmemberslist =redis.Smembers(msg.friend_or_group+"的群成员");
    for(const string &groupmember : groupmemberslist){
        //string name=redis.Hget(groupmember,"name");
        socket.mysend(groupmember);
    }
    socket.mysend("over");
    return;
}

void group_quit(StickyPacket socket,Message &msg){
    if(!redis.sismember(msg.friend_or_group+"的群成员",msg.uid)){
        socket.mysend("no_group_member");
        return;
    }

    //如果你是群主
    string owner_uid=redis.Hget(msg.friend_or_group,"群主");
    if(owner_uid==msg.uid){
        socket.mysend("quit");
        return;
    }

    //如果你是管理员
    if(redis.sismember(msg.friend_or_group+"的管理员",msg.uid)){
        redis.Srem(msg.friend_or_group+"的管理员",msg.uid);
        redis.Srem(msg.friend_or_group+"的高权限者",msg.uid);
        //要不要给群主和其他管理员发个消息呢？
        vector<string> all_managers_uid_list=redis.Smembers(msg.friend_or_group+"的高权限者");
        for(const string &all_managers_uid :all_managers_uid_list){
            if(all_managers_uid==msg.uid)  continue;

            string num=redis.Hget(all_managers_uid+"的未读消息","通知类消息");
            redis.hset(all_managers_uid+"的未读消息","通知类消息",to_string(stoi(num)+1));

            redis.Rpush(all_managers_uid+"的通知类消息",msg.uid+"(管理员)退出了"+msg.friend_or_group);

            if(online_users.find(all_managers_uid) !=online_users.end()){
                string notice_fd =redis.Hget(all_managers_uid,"消息fd");
                StickyPacket notice_socket(stoi(notice_fd));
                string notice=msg.uid+"(管理员）退出了"+msg.friend_or_group;
                notice_socket.mysend(notice);                                                                                                                                  
            }
        }

        redis.Srem(msg.friend_or_group+"的群成员",msg.uid);
        redis.Srem(msg.uid+"的群聊列表",msg.friend_or_group);
        
        return;
    }

    redis.Srem(msg.friend_or_group+"的群成员",msg.uid);
    redis.Srem(msg.uid+"的群聊列表",msg.friend_or_group);

    vector<string> all_managers_uid_list=redis.Smembers(msg.friend_or_group+"的高权限者");
    for(const string &all_managers_uid :all_managers_uid_list){
        string num=redis.Hget(all_managers_uid+"的未读消息","通知类消息");
        redis.hset(all_managers_uid+"的未读消息","通知类消息",to_string(stoi(num)+1));

        redis.Rpush(all_managers_uid+"的通知类消息",msg.uid+"退出了"+msg.friend_or_group);

        if(online_users.find(all_managers_uid) !=online_users.end()){
            string notice_fd =redis.Hget(all_managers_uid,"消息fd");
            StickyPacket notice_socket(stoi(notice_fd));
            string notice=msg.uid+"退出了"+msg.friend_or_group;
            notice_socket.mysend(notice);                                                                                                                                  
        }
    }  

    socket.mysend("ok");
    return;
}


void owner_add_managers(StickyPacket socket,Message &msg){
    string owner_uid=redis.Hget(msg.friend_or_group,"群主");
    if(owner_uid!=msg.uid){
        socket.mysend("quit");
        return;
    }

    if(redis.sismember(msg.friend_or_group+"的管理员",msg.other)){
        socket.mysend("have_exist");
        return;
    }

    redis.sadd(msg.friend_or_group+"的管理员",msg.other);
    redis.sadd(msg.friend_or_group+"的高权限者",msg.other);
    socket.mysend("ok");
    return;

}

void owner_del_managers(StickyPacket socket,Message &msg){
    string owner_uid=redis.Hget(msg.friend_or_group,"群主");
    if(owner_uid!=msg.uid){
        socket.mysend("quit");
        return;
    }

    if(!redis.sismember(msg.friend_or_group+"的管理员",msg.other)){
        socket.mysend("have_no_exist");
        return;
    }

    redis.Srem(msg.friend_or_group+"的管理员",msg.other);
    redis.Srem(msg.friend_or_group+"的高权限者",msg.other);

    socket.mysend("ok");
    return;


}

void owner_quit_group(StickyPacket socket,Message &msg){
    //如果你不是群主
    string owner_uid=redis.Hget(msg.friend_or_group,"群主");
    if(owner_uid!=msg.uid){
        socket.mysend("quit");
        return;
    }

    redis.Srem("群聊名字集合",msg.friend_or_group);
    redis.Srem(msg.uid+"创建的群聊",msg.friend_or_group);

    vector<string> groupmemberslist =redis.Smembers(msg.friend_or_group+"的群成员");
    for(const string &groupmember : groupmemberslist){
        redis.Srem(groupmember+"的群聊列表",msg.friend_or_group);
        //有必要删除用不到的键值吗
        //按没必要写
        string num=redis.Hget(groupmember+"的未读消息","通知类消息");
        redis.hset(groupmember+"的未读消息","通知类消息",to_string(stoi(num)+1));

        redis.Rpush(groupmember+"的通知类消息","群主已解散群聊"+msg.friend_or_group);

        if(online_users.find(groupmember) != online_users.end()){
            string notice_fd=redis.Hget(groupmember,"消息fd");
            StickyPacket notice_socket(stoi(notice_fd));
            string notice="群主已解散群聊"+msg.friend_or_group;
            notice_socket.mysend(notice);
        }
    }

    socket.mysend("ok");

    return;

}


void all_managers_del_members(StickyPacket socket,Message &msg){
    if(!redis.sismember(msg.friend_or_group+"的高权限者",msg.uid)){
        socket.mysend("no_high");
        return;
    }

    if(!redis.sismember(msg.friend_or_group+"的群成员",msg.other)){
        socket.mysend("no_exist");
        return;
    }

    if(!redis.sismember(msg.friend_or_group+"的高权限者",msg.other)){
        socket.mysend("quit");
        return;
    }
    
    redis.Srem(msg.other+"的群聊列表",msg.friend_or_group);
    redis.Srem(msg.friend_or_group+"的群成员",msg.other);

    vector<string> groupmemberslist =redis.Smembers(msg.friend_or_group+"的群成员");
    for(const string &groupmember : groupmemberslist){

       if(groupmember==msg.uid)  continue;

        
        string num=redis.Hget(groupmember+"的未读消息","通知类消息");
        redis.hset(groupmember+"的未读消息","通知类消息",to_string(stoi(num)+1));

        redis.Rpush(groupmember+"的通知类消息",msg.uid+"把"+msg.other+"踢出了群聊"+msg.friend_or_group);
        //群聊的聊天记录里显示
        //
        //
        //
        //
        //
        if(online_users.find(groupmember) != online_users.end()){
            string notice_fd=redis.Hget(groupmember,"消息fd");
            StickyPacket notice_socket(stoi(notice_fd));
            string notice=msg.uid+"把"+msg.other+"踢出了群聊"+msg.friend_or_group;
            notice_socket.mysend(notice);
        }
    }

    socket.mysend("ok");

    return;


}


void check_group_managers(StickyPacket socket,Message &msg){
    if(!redis.sismember("群聊名字集合",msg.friend_or_group)){
        socket.mysend("no_group_name");
        return;
    }

    vector<string> groupmemberslist =redis.Smembers(msg.friend_or_group+"的管理员");
    for(const string &groupmember : groupmemberslist){
        //string name=redis.Hget(groupmember,"name");
        socket.mysend(groupmember);
    }
    socket.mysend("over");
    return;
}


void group_chat(StickyPacket socket,Message &msg){
    if(!redis.sismember(msg.friend_or_group+"的群成员",msg.uid)){
        socket.mysend("no_exist");
        return;
    }

    redis.sadd(msg.friend_or_group+"的在线用户",msg.uid);

    vector<string>  chat_what = redis.Lrange(msg.uid+"与"+msg.friend_or_group+"的聊天记录");
    for(const string &notice :chat_what){
        socket.mysend(notice);
    }

    //处理未读消息中群聊消息
    string num1=redis.Hget(msg.uid+"的群聊消息",msg.friend_or_group);
    redis.hset(msg.uid+"的群聊消息",msg.friend_or_group,"0");

    string num2=redis.Hget(msg.uid+"的未读消息","群聊消息");
    redis.hset(msg.uid+"的未读消息","群聊消息",to_string(stoi(num2) - stoi(num1)));


    socket.mysend("over");
}

void group_daily_chat(StickyPacket socket,Message &msg){
    //对于我
    string my_notice="我："+msg.other;
    redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录",my_notice);
    string my_notice_fd=redis.Hget(msg.uid,"消息fd");
    StickyPacket my_notice_socket(stoi(my_notice_fd));
    //我肯定是群聊的在线用户，不需要特别颜色，也不需要处理未读消息
    my_notice_socket.mysend(my_notice);



    //对于他人
    string other_notice=msg.uid+":"+msg.other;
    vector<string> groupmemberslist =redis.Smembers(msg.friend_or_group+"的群成员");
    for(const string &groupmember : groupmemberslist){
        if(groupmember==msg.uid)  continue;

        redis.Rpush(groupmember+"与"+msg.friend_or_group+"的聊天记录",other_notice);

        //对于登录着的人
        if(online_users.find(groupmember)!=online_users.end()){
            string other_notice_fd=redis.Hget(groupmember,"消息fd");
            StickyPacket other_notice_socket(stoi(other_notice_fd));

            //对于在这个群聊界面的人
            if(redis.sismember(msg.friend_or_group+"的在线用户",groupmember)){
                other_notice_socket.mysend(other_notice);
            }else{
                other_notice_socket.mysend(RED "群聊"+msg.friend_or_group+"发来了一条消息"+RESET);
            }



        }
        //对于未登录的人
        else{
            string num1=redis.Hget(groupmember+"的群聊消息",msg.friend_or_group);
            redis.hset(groupmember+"的群聊消息",msg.friend_or_group,to_string(stoi(num1)+1));

            //string num2=redis.Hget(msg.uid+"的未读消息","群聊消息");
            redis.hset(msg.uid+"的未读消息","群聊消息",to_string(stoi(num1)+1));
        }
    }

    socket.mysend("ok");
}