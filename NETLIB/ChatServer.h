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
#include "Mutex.h"
#include "Callbacks.h"
#include "ChatContext.h"
#include "TcpServer.h"
#include <set>
#include <stdio.h>
#include <unistd.h>
#include <map>

class EventLoop;
class TcpServer;
class InetAddress;
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

private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n);
    void onRequest(const TcpConnectionPtr &conn, ChatContext & chatContext);
    TcpServer tcpServer_;
    typedef std::map<TcpConnectionPtr, std::string> UserConnMap;
    UserConnMap userConnMaps_;
    MutexLock mutex;
    ChatContext chatContext_;
};

#endif