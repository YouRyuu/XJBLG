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

StringPiece message(std::string("wodehahahhahaha"));

void test01()
{
    InetAddress listenaddr(8090);
    Socket listenfd(socket(AF_INET, SOCK_STREAM, 0));
    listenfd.bindAddress(listenaddr);
    listenfd.listen();
    for (;;)
    {
        char buff[1024];
        InetAddress peerAddr;
        int connfd = listenfd.accept(&peerAddr);
        printf("Main():conn from %s, port %d\n",
               inet_ntop(AF_INET, &peerAddr.getSock().sin_addr, buff, sizeof buff), ntohs(peerAddr.getSock().sin_port));
        close(connfd);
    }
}

void newConnection(int sockfd, const InetAddress &peerAddr) // for test02
{
    InetAddress peerAddr_;
    peerAddr_ = const_cast<InetAddress &>(peerAddr);
    char buff[1024];
    printf("Main():newconn from %s, port %d\n",
           inet_ntop(AF_INET, &peerAddr_.getSock().sin_addr, buff, sizeof buff), ntohs(peerAddr_.getSock().sin_port));
    close(sockfd);
}

void test02()
{
    InetAddress listenAddr(8090);
    EventLoop loop;
    Acceptor acceptor(&loop, listenAddr, true);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();
    loop.loop();
}

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, int size)
{
    printf("onMessage:tid=%d, recv %d bytes from [%s]:%s\n", CurrentThread::tid(),  size, conn->name().c_str(), buf->retrieveAllAsString().c_str());
}

void onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        printf("onConnection:tid=%d, new conn [%s]\n", CurrentThread::tid(), conn->name().c_str());
        conn->send(message);
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
    server.setThreadNum(5);
    server.start();
    loop.loop();
    return 0;
}