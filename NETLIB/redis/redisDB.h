#ifndef REDISDB_H
#define REDISDB_H

/* 利用redis存储相关数据
 * 1.存储离线消息：使用集合
 *  key：用户id
 * value：每一条消息
 * sadd userid message
 * 
 */ 

#include <sw/redis++/redis++.h>
#include <string>
#include <vector>
using namespace sw::redis;

class RedisDB
{
    public:
        void cacheMessage(std::string userID, std::string message);
        void getCacheMessage(std::vector<std::string> &v, std::string userID);
        void removeCacheMessage(std::string userID, std::string message);
        void removeAllCacheMessages(std::string userID);
        long long getCacheMessageSize(std::string userID);
    private:
        static Redis redis;
};

#endif