#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "../../net/TcpServer.h"
#include "../../net/Callbacks.h"
#include <functional>
class HttpResponse;
class HttpRequest;
class HttpServer
{
public:
    typedef std::function<void(const HttpRequest &, HttpResponse *)> HttpCallback;
    HttpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &name);

    EventLoop *getLoop() const
    {
        return tcpServer_.getLoop();
    }

    void setHttpCallback(const HttpCallback &cb)
    {
        httpCallback_ = cb;
    }

    void setThreadNum(int numThreads)
    {
        tcpServer_.setThreadNum(numThreads);
    }

    void start();

private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n);
    void onRequest(const TcpConnectionPtr &conn, const HttpRequest& req);
    TcpServer tcpServer_;
    HttpCallback httpCallback_;
};

#endif