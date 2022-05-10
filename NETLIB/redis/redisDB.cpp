#include "redisDB.h"

sw::redis::Redis RedisDB::redis = Redis("tcp://127.0.0.1:6379");

void RedisDB::cacheMessage(std::string userID, std::string message)
{
    redis.sadd(userID, message);
}

void RedisDB::getCacheMessage(std::vector<std::string> &v, std::string userID)
{
    if(redis.exists(userID))
    {
        redis.smembers(userID, std::back_inserter(v));
    }
}

long long RedisDB::getCacheMessageSize(std::string userID)
{
    return redis.scard(userID);
}

void RedisDB::removeAllCacheMessages(std::string userID)
{
    redis.del(userID);
}

void RedisDB::removeCacheMessage(std::string userID, std::string message)
{
    if(redis.exists(userID))
    {
        redis.srem(userID, message);
    }
}
