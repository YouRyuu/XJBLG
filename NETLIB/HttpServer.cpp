#include "HttpServer.h"
#include "HttpResponse.h"
#include "Callbacks.h"

void defaultHttpCallback(HttpResponse *resp)
{
    resp->setStatusCode(HttpResponse::k404);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const std::string &name)
    : tcpServer_(loop, listenAddr, name),
      httpCallback_(defaultHttpCallback)
{
    tcpServer_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    tcpServer_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::start()
{
    tcpServer_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        printf("onConnection, new conn [%s]\n", conn->name().c_str());
        //conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        // conn->shutdown();
    }
    else
    {
        printf("onConnection:, conn[%s] is down\n", conn->name().c_str());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buf, int n)
{
    printf("onMessage:recv %d bytes from [%s]:%s\n", n, conn->name().c_str(), buf->retrieveAllAsString().c_str());
    onRequest(conn);
}

void HttpServer::onRequest(const TcpConnectionPtr &conn)
{
    HttpResponse response(0);
    httpCallback_(&response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}