#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include "InetAddress.h"
#include "Socket.h"
#include "Eventloop.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "Buffer.h"
#include "StringPiece.h"
#include "ThreadPool.h"
#include "CurrentThread.h"


char *s = "HTTP/1.1 200 OK\r\nContent-Length: 91\r\nConnecion: Keep-Alive\r\nContent-Type: text/html\r\nServer: Muduo\r\n\r\n<html><head><title>This is title</title></head><body><h1>Hello</h1>Now is xxx</body></html>";


void onMessage(const TcpConnectionPtr &conn, Buffer *buf, int size)
{
    printf("onMessage:tid=%d, recv %d bytes from [%s]:%s\n", CurrentThread::tid(),  size, conn->name().c_str(), buf->retrieveAllAsString().c_str());
    conn->send(s);
}

void onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        printf("onConnection:tid=%d, new conn [%s]\n", CurrentThread::tid(), conn->name().c_str());
        //conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        // conn->shutdown();
    }
    else
    {
        printf("onConnection:tid=%d, conn[%s] is down\n", CurrentThread::tid(), conn->name().c_str());
    }
}

void test03()
{
}

int main()
{
    printf("main():tid=%d\n", getpid());
    InetAddress listenAddr(9898);
    EventLoop loop;
    TcpServer server(&loop, listenAddr, "myserver");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setThreadNum(0);
    server.start();
    loop.loop();
    return 0;
}