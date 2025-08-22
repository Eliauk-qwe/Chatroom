#include "server.hpp"

void friend_apply_agree(StickyPacket socket, Message &msg)
{

    // redis.Hdel(msg.uid+"的好友申请",msg.friend_or_group);
    if (!redis.Hexists(msg.uid + "的新的朋友", msg.friend_or_group))
    {
        socket.mysend("no_exist");
        return;
    }

    if (!redis.sismember("用户ID集合", msg.friend_or_group))
    {
        socket.mysend("no");
        return;
    }

    string name1 = redis.Hget(msg.uid, "name");
    string name2 = redis.Hget(msg.friend_or_group, "name");

    redis.hset(msg.uid + "的好友列表", msg.friend_or_group, name2);
    redis.hset(msg.friend_or_group + "的好友列表", msg.uid, name1);
    if (!redis.Exists(msg.uid + "与" + msg.friend_or_group + "的聊天记录"))
    {

        redis.Rpush(msg.uid + "与" + msg.friend_or_group + "的聊天记录", "---------" + name1 + "与" + name2 + "的聊天界面--------");
        redis.Rpush(msg.uid + "与" + msg.friend_or_group + "的聊天记录", "你通过了" + name2 + "的好友申请,你们可以聊天了");
    }

    if (!redis.Exists(msg.friend_or_group + "与" + msg.uid + "的聊天记录"))
    {
        redis.Rpush(msg.friend_or_group + "与" + msg.uid + "的聊天记录", "---------" + name2 + "与" + name1 + "的聊天界面--------");
        redis.Rpush(msg.friend_or_group + "与" + msg.uid + "的聊天记录", name1 + "通过了你的好友申请,你们可以聊天了");
    }

    /*string num1 = redis.Hget(msg.friend_or_group + "的未读消息", "好友消息");
    redis.hset(msg.friend_or_group + "的未读消息", "好友消息", (to_string(stoi(num1) + 1)));
    redis.hset(msg.friend_or_group+"的好友消息",msg.uid,"1");


    redis.hset(msg.uid+"的好友消息",msg.friend_or_group,"0");*/

    redis.Rpush(msg.friend_or_group + "的通知类消息", msg.uid + ":" + name1 + "通过了你的好友申请");
    string num2 = redis.Hget(msg.friend_or_group + "的未读消息", "通知类消息");
    redis.hset(msg.friend_or_group + "的未读消息", "通知类消息", (to_string(stoi(num2) + 1)));

    if (online_users.find(msg.friend_or_group) != online_users.end())
    {
        string friend_fd = redis.Hget(msg.friend_or_group, "消息fd");
        StickyPacket friendsocket(stoi(friend_fd));
        string notice = msg.uid + ":" + name1 + "通过了你的好友申请";
        friendsocket.mysend(QING + notice + RESET);
    }
    redis.Hdel(msg.uid + "的新的朋友", msg.friend_or_group);

    socket.mysend("ok");
    return;
}

void friend_apply_refuse(StickyPacket socket, Message &msg)
{
    if (!redis.Hexists(msg.uid + "的好友申请", msg.friend_or_group))
    {
        socket.mysend("no_exist");
        return;
    }
    if (!redis.sismember("用户ID集合", msg.friend_or_group))
    {
        socket.mysend("no");
        return;
    }

    redis.Hdel(msg.uid + "的新的朋友", msg.friend_or_group);
    /*string num1 = redis.Hget(msg.uid + "的未读消息", "新的朋友");
    redis.hset(msg.uid + "的未读消息", "新的朋友", (to_string(stoi(num1) - 1)));*/

    // redis.hset(msg.uid+"的好友列表",msg.friend_or_group,"ok");
    // redis.hset(msg.friend_or_group+"的好友列表",msg.uid,"ok");

    // redis.Rpush(msg.uid+"与"+msg.friend_or_group+"的聊天记录","---------");
    // redis.Rpush(msg.friend_or_group+"与"+msg.uid+"的聊天记录","---------");
    // redis.Rpush(msg.friend_or_group+"与"+msg.uid+"的聊天记录",msg.uid+"通过了你的好友申请");
    string name1 = redis.Hget(msg.uid, "name");
    string name2 = redis.Hget(msg.friend_or_group, "name");

    string notice = msg.uid + ":" + name1 + "拒绝了你的好友申请";
    redis.Rpush(msg.friend_or_group + "的通知类消息", notice);
    string num2 = redis.Hget(msg.friend_or_group + "的未读消息", "通知类消息");
    redis.hset(msg.friend_or_group + "的未读消息", "通知类消息", (to_string(stoi(num2) + 1)));

    if (online_users.find(msg.friend_or_group) != online_users.end())
    {
        string friend_fd = redis.Hget(msg.friend_or_group, "消息fd");
        StickyPacket friendsocket(stoi(friend_fd));
        friendsocket.mysend(QING + notice + RESET);
    }

    socket.mysend("OK");
    return;
}

void check_friend_apply(StickyPacket socket, Message &msg)
{
    if (!redis.Hlen(msg.uid + "的新的朋友"))
    {
        socket.mysend("no");
        return;
    }

    vector<string> friendapplylist = redis.Hgetall(msg.uid + "的新的朋友");
    for (int i = 0; i < friendapplylist.size(); i = i + 2)
    {
        string notice = friendapplylist[i] + "    " + friendapplylist[i + 1];
        socket.mysend(PLUSWHITE + notice + RESET);
    }

    // string num1 = redis.Hget(msg.uid + "的未读消息", "新的朋友");
    redis.hset(msg.uid + "的未读消息", "新的朋友", "0");

    socket.mysend("over");
}