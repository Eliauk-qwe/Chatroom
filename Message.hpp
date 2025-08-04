#ifndef _MESSAGE_
#define _MESSAGE_
#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// 使用明确的类型别名
using std::string;
using std::vector;
using json = nlohmann::json;

class Message {
public:
    string uid = "";
    string name = "";
    string pass = "";
    string friend_or_group = "";
    string phone = "";
    string other="";
    int flag = 0;
    vector<string> para = {};

    // 默认构造函数
    Message() = default;
    
    // 构造函数列表
    //注册
    Message(int flag,string phone, string pass, string name) 
        : flag(flag),phone(phone), pass(pass), name(name) {}
    //登录
    Message(string phone, string uid, string pass, int flag)
        : phone(phone),uid(uid),pass(pass), flag(flag) {}
    
    Message(string uid, string phone, int flag) 
        : uid(uid), phone(phone), flag(flag) {}
    
    // 查看好友申请  群聊列表
    Message(string uid, int flag) 
        : uid(uid), flag(flag) {}
    //好友申请  申请加群   同意群聊申请  群主添加管理员  群聊日常聊天
    Message(string uid,int flag,string friend_or_group,string other)
        :uid(uid),flag(flag),friend_or_group(friend_or_group),other(other){}

    //好友删除   同意添加好友   群聊创建  成员退出群聊   群聊聊天
    Message(string uid,int flag,string friend_or_group)
        :uid(uid),flag(flag),friend_or_group(friend_or_group){}

    Message(int flag,string friend_or_group)
        :flag(flag),friend_or_group(friend_or_group){}

    //发送文件
    Message(string uid,vector<string> para,int flag)
         :uid(uid),para(para),flag(flag){}

    Message(string uid,string friend_or_group,vector<string> para,int flag,string other)
         :uid(uid),friend_or_group(friend_or_group),para(para),flag(flag),other(other){}


    // JSON转换方法
    void Json_to_s(const string &jsr)
    {
        json js = json::parse(jsr);

        uid = js["uid"];
        name = js["name"];
        pass = js["pass"];
        friend_or_group = js["friend_or_group"];
        flag = js["flag"];
        phone = js["phone"];
        para = js["para"];
        other = js["other"];
    }

    string S_to_json() {
        
        json js;
        js["uid"] = uid;
        js["name"] = name;
        js["pass"] = pass;
        js["friend_or_group"] = friend_or_group;
        js["flag"] = flag;
        js["phone"] = phone;
        js["para"] = para;
        js["other"] = other;
        return js.dump();
    }
};

#endif