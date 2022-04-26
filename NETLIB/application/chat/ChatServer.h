#ifndef CHATSERVER_H
#define CHATSERVER_H

//聊天服务器
/*
服务器端需要做的工作：
    解码消息：
        消息类型：
            注册、登录、一个客户发送给另一个客户的消息、加好友、删好友、查找用户信息
            当一个新的连接到来时（OnConnection），它是登录消息（暂时不考虑注册），这时候需要维护一个在线用户和它的conn对应关系
            因为客户给另一个客户发消息是通过server中转的，server需要知道用户ID
*/
#include "../../net/Mutex.h"
#include "../../net/Callbacks.h"
#include "ChatContext.h"
#include "../../net/TcpServer.h"
#include <set>
#include <stdio.h>
#include <unistd.h>
#include <map>
#include "ChatModel.h"

class EventLoop;
class TcpServer;
class InetAddress;
class Buffer;
class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr, const std::string &name);

    void setThreadNum(int numThreads)
    {
        tcpServer_.setThreadNum(numThreads);
    }

    void start()
    {
        tcpServer_.start();
    }
    void sendMessage(const TcpConnectionPtr &conn, std::string msg);
    void sendMessage(const TcpConnectionPtr &conn, std::string code, std::string sender, std::string recver, std::string time, std::string body);
    void cacheMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body);
    //功能函数：获取用户在线状态、获取用户的好友列表、获取用户的信息
    bool getUserState(std::string userId);
    std::vector<std::pair<std::string, std::string>> getUserFriends(std::string userId);
    std::string userFriendsToString(std::vector<std::pair<std::string, std::string>> /*&*/friends);
private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n);
    void onContextMessage(const TcpConnectionPtr conn, ChatContext chatContext, int n);
    void onRequest(const TcpConnectionPtr &conn, ChatContext & chatContext);
    bool loginValid(const std::string userid, const std::string password);
    const static int systemId = 10000000;
    TcpServer tcpServer_;
    //typedef std::map<TcpConnectionPtr, std::string> UserConnMap;
    typedef std::map<std::string, TcpConnectionPtr> UserConnMap;
    typedef std::map<std::string, std::vector<std::string>> CachedMessages;      //缓存的消息
    UserConnMap userConnMaps_;
    CachedMessages cachedMessages_;
    MutexLock mutex;
    ChatContext chatContext_;
    ChatModel model_;
};

#endif