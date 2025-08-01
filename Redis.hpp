#ifndef _REDIS_H_
#define _REDIS_H_

#include <hiredis/hiredis.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


class Redis{
private:
    redisContext  *con ;
    
           

public:
    Redis(){
        con=redisConnect("127.0.0.1",6379);
        if(con == nullptr){
            cout << "Can not allocate redis context" << endl;
        }
        else if(con -> err){
            cout << con->errstr <<endl;
            redisFree(con);
        }
        else{
            cout << "Connect to redis successfully!" << endl;
        }
    }

    ~Redis(){
        redisFree(con);
    }

    //集合SADD  在集合key中添加member元素
    bool sadd (const string &key,const string &member){
        redisReply *reply = (redisReply *)redisCommand(con,"SADD %s %s",key.c_str(),member.c_str());

        if(reply == nullptr){
            fprintf(stderr,"SADD fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"SADD错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER && reply->integer==1){
            printf("SADD成功: %s",reply->str);
            freeReplyObject(reply);
            return true;
        }
        else if(reply->type ==REDIS_REPLY_INTEGER && reply->integer==0){
            printf("集合中已存在该元素: %s",reply->str);
            freeReplyObject(reply);
            return false;
        }

        freeReplyObject(reply);
        return false;
    }

    //集合SCARD  获取集合key中元素个数
    int scard (const string &key){
        redisReply *reply = (redisReply *)redisCommand(con,"SCARD %s",key.c_str());

        if(reply == nullptr){
            fprintf(stderr,"SCARD fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"SCARD错误 :%s\n",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER ){
            printf("SCARD成功: %s\n",reply->str);
            cout << key<<"中元素个数为"<< reply->integer<<endl;
            int num=reply->integer;
            freeReplyObject(reply);
            return num;
        }

        freeReplyObject(reply);
        return reply->integer;
    }


    //哈希HSET  设置  1001 name wly |||| key field alue  返回值为新增的字段数
    int hset (const string &key,const string &field,const string &value){
        redisReply *reply = (redisReply *)redisCommand(con,"HSET %s %s %s",key.c_str(),field.c_str(),value.c_str());

        if(reply == nullptr){
            fprintf(stderr,"HSET fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"HSET错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER ){
            printf("HSET成功: %s",reply->str);
            cout<<reply->integer<<endl;
            freeReplyObject(reply);
            return reply->integer;
        }

        freeReplyObject(reply);
        return reply->integer;
    }
    //判断集合中的成员在不在
    bool sismember (const string &key,const string &member){
        redisReply *reply =(redisReply *) redisCommand(con,"SISMEMBER %s %s",key.c_str(),member.c_str());

        if(reply == nullptr){
            fprintf(stderr,"SISMEMBER fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"SISMEMBER错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER ){
            printf("SISMEMBER成功");
            int num=reply->integer;
            freeReplyObject(reply);
            return num;
        }

        freeReplyObject(reply);
        return false;
    }

    string Hget (const string &key,const string &field){
        redisReply *reply = (redisReply *)redisCommand(con,"HGET %s %s",key.c_str(),field.c_str());

        if(reply == nullptr){
            fprintf(stderr,"HGET fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"HGET错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_STRING ){
            printf("HGET成功:%s",reply->str);
            string recv=reply->str;
            freeReplyObject(reply);
            return recv;
        }

        freeReplyObject(reply);

        return "";
        
        
    }

    vector<std::string> Hkeys(const string &key,const string &field){
        string listkey=key+field;
        vector<string> list;
        redisReply *reply = (redisReply *)redisCommand(con,"HKEYS %s %s",key.c_str(),field.c_str());

        if(reply == nullptr){
            fprintf(stderr,"HKEYS fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"HKEYS错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_ARRAY ){
            for(size_t i=0; i<reply->elements;i++){
                list.push_back(reply->element[i]->str);
            }
        }

        freeReplyObject(reply);
        return list;

    }

    bool Hdel (const string &key,const string &member){
        redisReply *reply = (redisReply *)redisCommand(con,"HDEL %s %s",key.c_str(),member.c_str());

        if(reply == nullptr){
            fprintf(stderr,"HDEL fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"HDEL错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER && reply->integer==1){
            printf("HDEL成功");
            freeReplyObject(reply);
            return true;
        }

        freeReplyObject(reply);
        return false;
    }


    bool del (const string &key){
        redisReply *reply = (redisReply *)redisCommand(con,"DEL %s ",key.c_str());

        if(reply == nullptr){
            fprintf(stderr,"DEL fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"DEL错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER && reply->integer==1){
            printf("DEL成功");
            freeReplyObject(reply);
            return true;
        }

        freeReplyObject(reply);
        return false;
    }


    bool Srem (const string &key,const string &member){
        redisReply *reply = (redisReply *)redisCommand(con,"SREM %s %s",key.c_str(),member.c_str());

        if(reply == nullptr){
            fprintf(stderr,"SREM fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"SREM错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER && reply->integer==1){
            printf("SREM成功");
            freeReplyObject(reply);
            return true;
        }

        freeReplyObject(reply);
        return false;
    }


    string Lindex(const string &key, int index)
    {
        redisReply *reply = (redisReply *)redisCommand(this->con, "LINDEX %s %d", key.c_str(), index);
        if (reply != nullptr && reply->type == REDIS_REPLY_STRING)
        {
            string value = reply->str;
            freeReplyObject(reply);
            return value;
        }
        return "";
    }

    int Hlen(const string &key){
        redisReply *reply = (redisReply *)redisCommand(this->con, "HLEN %s ", key.c_str());
        if(reply == nullptr){
            fprintf(stderr,"HLEN fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"HLEN错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER ){
            printf("HLEN成功");
            int count=reply->integer;
            freeReplyObject(reply);
            return count;
        }

        freeReplyObject(reply);
        return 0;
    }


    vector<std::string> Hgetall(const string &key)
    {
        
        vector<string> friendlist;
        redisReply *reply = (redisReply *)redisCommand(this->con, "HGETALL %s", key.c_str());
        if (reply != nullptr && reply->type == REDIS_REPLY_ARRAY)
        {
            for (size_t i = 0; i < reply->elements; ++i)
            {
                friendlist.push_back(reply->element[i]->str);
            }
            freeReplyObject(reply);
        }
        return friendlist;
    }

    bool Hexists (const string &key,const string &field){
        redisReply *reply =(redisReply *) redisCommand(con,"HEXISTS %s %s",key.c_str(),field.c_str());

        if(reply == nullptr){
            fprintf(stderr,"HEXISTS fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"HEXISTS错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER && reply->integer==1){
            printf("HEXISTS成功");
            freeReplyObject(reply);
            return true;
        }

        freeReplyObject(reply);
        return false;
    }


    bool Exists (const string &key){
        redisReply *reply = (redisReply *)redisCommand(con,"EXISTS %s ",key.c_str());

        if(reply == nullptr){
            fprintf(stderr,"EXISTS fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"EXISTS错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER && reply->integer==1){
            printf("EXISTS成功\n");
            freeReplyObject(reply);
            return true;
        }

        freeReplyObject(reply);
        return false;
    }

    vector<string> Smembers(const string &key)
    {
        vector<string> members;
        redisReply *reply = (redisReply *)redisCommand(this->con, "SMEMBERS %s", key.c_str());

        if(reply == nullptr){
            fprintf(stderr,"EXISTS fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"EXISTS错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_ARRAY ){
            printf("SMEMBERS成功\n");
            for (size_t i = 0; i < reply->elements; ++i)
            {
                cout<<"i:"<<i<<endl;
                members.push_back(reply->element[i]->str);
            }
            printf("123\n");
            freeReplyObject(reply);
            printf("456\n");
            return members;
        }

        freeReplyObject(reply);
        return {};


    }


    

    int Rpush (const string &key,const string &field){
        redisReply *reply =(redisReply *) redisCommand(con,"RPUSH %s %s",key.c_str(),field.c_str());

        if(reply == nullptr){
            fprintf(stderr,"RPUSH fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"RPUSH错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER && reply->integer>0){
            printf("RPUSH成功\n");
            int num=reply->integer;
            freeReplyObject(reply);
            return num;
        }

        freeReplyObject(reply);
        return 0;
    }


    vector<string> Lrange (const string &key){
        redisReply *reply =(redisReply *) redisCommand(con,"LRANGE %s 0 -1",key.c_str());

        vector<string> recv;
        if(reply == nullptr){
            fprintf(stderr,"LRANGE fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"LRANGE错误 :%s",reply->str);
        }else if(reply->type == REDIS_REPLY_ARRAY){
            for(size_t i=0;i<reply->elements;i++){
                if(reply->element[i]->type == REDIS_REPLY_STRING){
                    recv.push_back(reply->element[i]->str);
                }
            }
            freeReplyObject(reply);
            return recv;

        }

        freeReplyObject(reply);
        return {};
    }

    string incr(const string& key) {
        redisReply* reply = (redisReply*)redisCommand(con, "INCR %s", key.c_str());
        string res = to_string(reply->integer);
        freeReplyObject(reply);
        return res;
    }

    // SET 命令封装
    bool set(const string& key, const string& value) {
        // 构造命令: SET key value
        redisReply* reply = (redisReply*)redisCommand(con, "SET %s %s", key.c_str(), value.c_str());
        
        if (reply == nullptr) {
            cerr << "Redis SET command failed: NULL reply" << endl;
            return false;
        }
        
        if (reply->type == REDIS_REPLY_ERROR) {
            cerr << "Redis SET command failed: " << reply->str << endl;
            freeReplyObject(reply);
            return false;
        }
        
        // SET 命令成功返回 OK
        bool success = (reply->type == REDIS_REPLY_STATUS && 
                       string(reply->str) == "OK");
        
        freeReplyObject(reply);
        return success;
    }



    int Llen(const string &key){
        redisReply *reply = (redisReply *)redisCommand(this->con, "LLEN %s ", key.c_str());
        if(reply == nullptr){
            fprintf(stderr,"HLEN fail\n");
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            fprintf(stderr,"HLEN错误 :%s",reply->str);
        }
        else if(reply->type ==REDIS_REPLY_INTEGER ){
            printf("HLEN成功");
            int count=reply->integer;
            freeReplyObject(reply);
            return count;
        }

        freeReplyObject(reply);
        return 0;
    }


    

    




};



#endif