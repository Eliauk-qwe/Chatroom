#include "server.hpp"

//==========================好友聊天=======================================
void init_friend_message_process(){
    thread friend_process(friend_message_process);
    friend_process.detach();
}


void friend_message_process(){

    // 创建独立的Redis连接
    Redis local_redis;

    while(1){
        // 从消息队列中获取消息
        string message = local_redis.dequeue_message("friend_message_queue");
        if (message.empty())  continue;


        // 解析消息
        // 格式: sender_uid|receiver_uid|message_content

        size_t pos1=message.find('|');
        size_t pos2=message.find('|',pos1+1);

        if(pos1==string::npos || pos2==string::npos)  continue;

        string sender_uid=message.substr(0,pos1);
        string recver_uid=message.substr(pos1+1,pos2-pos1-1);
        string content =message.substr(pos2+1);

        //处理消息
        process_friendchat_message(local_redis,sender_uid,recver_uid,content);


    }

}

void process_friendchat_message(Redis& local_redis,string &sender_uid,string &recver_uid,string &content){
    //生成锁键和值
    string lock_key=sender_uid+"-"+recver_uid;
    string lock_value=local_redis.generate_lock_value();

    if(!local_redis.acquire_lock(lock_key,lock_value,10)){
        cerr<<"获取锁失败"<<lock_key<<endl;
        return;
    }

    //==================已进入redis锁的作用域===============

    // 获取两个客户端的socket
    string fd1 = local_redis.Hget(sender_uid, "消息fd");
    string fd2 = local_redis.Hget(recver_uid, "消息fd");
    StickyPacket fd1_socket(fd1.empty() ? -1 : stoi(fd1));
    StickyPacket fd2_socket(fd2.empty() ? -1 : stoi(fd2));


    // 正常聊天处理
        string notice1 = PLUSWHITE "我：" RESET + content;
        local_redis.Rpush(sender_uid + "与" + recver_uid + "的聊天记录", notice1);

        string name = local_redis.Hget(sender_uid, "name");
        string notice2 = ZI + name + ":" RESET + content;
        local_redis.Rpush(recver_uid + "与" + sender_uid + "的聊天记录", notice2);

        // 发送消息给发送方
        if (fd1_socket.getfd() != -1) {
            fd1_socket.mysend(notice1);
        }

        // 发送消息给接收方
        if (fd2_socket.getfd() != -1) {
            // 检查接收方是否正在与发送方聊天
            if((online_users.find(recver_uid)!=online_users.end())  &&  (local_redis.Hget(recver_uid,"聊天对象") == sender_uid)){
                fd2_socket.mysend(notice2);
            }
            else if ((online_users.find(recver_uid) != online_users.end()) && (local_redis.Hget(recver_uid, "聊天对象") != sender_uid))
            {
                fd2_socket.mysend(QING + sender_uid + ":" + name + "给你发了一条消息" + RESET);
            }
        }

        // 释放锁
        local_redis.release_lock(lock_key, lock_value);
}
