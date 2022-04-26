#include "HttpServer.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "HttpContext.h"
#include "../../net/Callbacks.h"

void defaultHttpCallback(const HttpRequest &, HttpResponse *resp)
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
        conn->setContext(HttpContext());
    }
    else
    {
        printf("onConnection:, conn[%s] is down\n", conn->name().c_str());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buf, int n)
{
    //printf("-----\nonMessage:recv %d bytes from [%s]:\n%s-------\n", n, conn->name().c_str(), buf->getAllStringFromBuffer().c_str());
    HttpContext *context = conn->getMutableContext();
    if (!context->parseRequest(buf))
    {
        //请求解析失败,关闭连接
        printf("HttpServer::onMessage close conn\n");
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
    if(context->gotAll())
    {
        //printf("HttpServer::onMessage parse succ\n");
        //解析成功
        onRequest(conn, context->request());
        context->reset();
    }
    
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req)
{
    const std::string &connection = req.getHeader("Connection");
    // bool close = connection == "close" ||
    //              (req.getVersion() == HttpRequest::Http10 && connection != "Keep-Alive");
    HttpResponse response(0);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.closeConnection())
    {
        conn->shutdown();
    }
    printf("send succ\n");
}