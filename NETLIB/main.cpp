#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string>
#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/Eventloop.h"
#include "net/Acceptor.h"
#include "net/TcpConnection.h"
#include "net/TcpServer.h"
#include "net/Buffer.h"
#include "net/StringPiece.h"
#include "net/ThreadPool.h"
#include "net/CurrentThread.h"
#include "application/http/HttpResponse.h"
#include "application/http/HttpServer.h"
#include "application/http/HttpRequest.h"
#include "application/chat/ChatServer.h"

////int filefd = open("photo2.jpg", O_RDONLY);
//Buffer photo;

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, int size)
{
    printf("onMessage:tid=%d, recv %d bytes from [%s]:%s\n", CurrentThread::tid(), size, conn->name().c_str(), buf->retrieveAllAsString().c_str());
    // conn->send(s);
}

void onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        printf("onConnection:tid=%d, new conn [%s]\n", CurrentThread::tid(), conn->name().c_str());
    }
    else
    {
        printf("onConnection:tid=%d, conn[%s] is down\n", CurrentThread::tid(), conn->name().c_str());
    }
}

void test05()
{
    printf("main():tid=%d\n", getpid());
    InetAddress listenAddr(9898);
    EventLoop loop;
    TcpServer server(&loop, listenAddr, "myserver");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setThreadNum(3);
    server.start();
    loop.loop();
}
std::string name;

void onRequest(const HttpRequest &req, HttpResponse *resp)
{
    int method = req.getMethod();
    //std::cout << req.methodString() << std::endl;
    std::string path = req.getPath();
    //std::cout<<path<<std::endl;
    // if(filefd < 0)
    // {
    //     std::cout<<"open photo1.jpg error"<<std::endl;
    //     exit(1);
    // }
    if (path == "/" && method == HttpRequest::Method::GET)
    {
        name = "guest";
        resp->setStatusCode(HttpResponse::k200ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "MyServer");
        resp->setBody((std::string("<!DOCTYPE html>"
                                   "<html lang=\"en\"> "
                                   "<head>"
                                   " <meta charset=\"UTF-8\">"
                                   "<title>Title</title>"
                                   "</head>"
                                   "<body>"
                                   "hello, ") +
                       name +
                       std::string("<form method=\"post\">"
                                   "       <div >"
                                   " <input type=\"text\" placeholder=\"请输入手机号\" name=\"tel\"/>"
                                   "  <input type=\"password\" placeholder=\"请输入密码\" name=\"password\"/>"
                                   "      </div>"
                                   "  <div class=\"one\">"
                                   "    <table>"
                                   "    <tr>"
                                   "   <td><button type=\"submit\" >登录</button> </td>"
                                   "    <td><button>注册</button></td>"
                                   "     </tr>"
                                   "    </table>"
                                   " </div>"
                                   "</body>"
                                   "</html>"))
                          .c_str());
    }
    else if (path == "/" && method == HttpRequest::Method::POST)
    {
        //std::cout<<"in path=/ and method=post:"<<method<<std::endl;
        //读取POST的body数据,格式:pam1=value1&pam2=value2...
        name = req.getBody();
        //std::cout << name << std::endl;
        resp->setStatusCode(HttpResponse::k200ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "MyServer");
        resp->setBody((std::string("<!DOCTYPE html>"
                                   "<html lang=\"en\"> "
                                   "<head>"
                                   " <meta charset=\"UTF-8\">"
                                   "<title>Title</title>"
                                   "</head>"
                                   "<body>"
                                   "hello, ") +
                       name)
                          .c_str() +
                      std::string("</body>"));
    }
    else
    {
        //int filefd = open("photo2.jpg", O_RDONLY);
        //Buffer photo;
        //if(filefd < 0)
        //{
       //     std::cout<<"filefd error"<<std::endl;
       //     exit(1);
        //}
        //photo.bufReadFd(filefd);
        
        resp->setStatusCode(HttpResponse::k200ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        //resp->setContentType("image/png");
        resp->addHeader("Server", "Muduo");
        resp->setBody("<html><head><title>This is title</title></head>"
                     "<body><h1>Hello</h1><img src=\"img.png\"></body></html>");
        //resp->setBody(photo.retrieveAllAsString());
        //close(filefd);
    }
}
void onRequest2(const HttpRequest &req, HttpResponse *resp)
{
    //std::cout << "Header:" << req.methodString() << " " << req.getPath() << std::endl;
    // const std::map<std::string, std::string> &headers = req.getHeaders();
    // for (const auto &item : headers)
    // {
    //     std::cout << item.first << ":" << item.second << std::endl;
    // }
    resp->setStatusCode(HttpResponse::k200ok);
    resp->setStatusMessage("OK");
    resp->setContentType("text/html");
    //resp->setContentType("image/png");
    resp->addHeader("Server", "Muduo");
     resp->setBody("<html><head><title>This is title</title></head>"
                  "<body><h1>Hello</h1>Now is xxx</body></html>");
}

void testHttp()
{
    int numThreads = 4;
    EventLoop loop;
    HttpServer server(&loop, InetAddress(9898), "dummy");
    server.setHttpCallback(onRequest);
    server.setThreadNum(numThreads);
    server.start();
    loop.loop();
}

void testChat()
{
    EventLoop loop;
    ChatServer server(&loop, InetAddress(6666), "ChatServer");
    server.start();
    loop.loop();
}

int main(int argc, char *argv[])
{
    testChat();
    //testHttp();
    return 0;
}