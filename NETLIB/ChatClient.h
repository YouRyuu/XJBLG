#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include "Callbacks.h"
#include "Mutex.h"
#include "TcpClient.h"
#include "ChatContext.h"

class EventLoop;
class InetAddress;

class ChatClient
{
    public:
        ChatClient(EventLoop *loop, const InetAddress &serverAddr, const std::string &nameArg);
        ~ChatClient();
        void connect();
        void disconnect();
        void setUserId(std::string userId)
        {
            userId_ = userId;
        }
        void send(std::string &line);
    private:
        //连接建立完成之后会调用这个函数
        void onConnection(const TcpConnectionPtr &conn);
        //收到信息时候调用这个函数
        void onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n);
        ChatContext chatContext_;
        TcpClient client_;
        TcpConnectionPtr connection_;
        MutexLock lock;
        std::string userId_;
};

#endif