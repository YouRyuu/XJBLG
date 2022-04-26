#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "TcpConnection.h"
#include <memory>
#include <map>

class Acceptor;
class EventLoop;
class InetAddress;
class EventLoopThreadPool;
class TcpServer
{
    public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name);
    ~TcpServer();

    const std::string& name() {
        return name_;
    }

    void setThreadNum(int nums);
    void setThreadInitCallback(const ThreadInitCallback& cb)
    {
        threadInitCallback_ = cb;
    }

    std::shared_ptr<EventLoopThreadPool> threadPool()
    {
        return threadPool_;
    }

    void start();

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    EventLoop* getLoop() const { return loop_; }

    private:
        void newConnection(int sockfd, const InetAddress& peerAddr);
        void removeConnection(const TcpConnectionPtr& conn);
        void removeConnectionInLoop(const TcpConnectionPtr& conn);
        typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
        std::string ipPort;
        EventLoop* loop_;
        const std::string name_;
        std::unique_ptr<Acceptor> acceptor_;
        std::shared_ptr<EventLoopThreadPool> threadPool_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        int nextConnId_;
        ConnectionMap connections_;
        ThreadInitCallback threadInitCallback_;
};

#endif