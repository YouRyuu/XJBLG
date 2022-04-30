#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include "../../net/Callbacks.h"
#include "../../net/Mutex.h"
#include "../../net/TcpClient.h"
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
        void setUserPassword(std::string password)
        {
            password_ = password;
        }
        void sendMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body);
        void send(std::string &line);
        void sendMessageTo(std::string message, std::string recver);
        std::string encodeMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body);
    private:
        //连接建立完成之后会调用这个函数
        void onConnection(const TcpConnectionPtr &conn);
        //收到信息时候调用这个函数
        void onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n);
        void onContextMessage(const TcpConnectionPtr conn, ChatContext chatContext, int n);
        ChatContext chatContext_;
        TcpClient client_;
        TcpConnectionPtr connection_;
        MutexLock lock;
        std::string userId_;
        std::string password_;
};

#endif