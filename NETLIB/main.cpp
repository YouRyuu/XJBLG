#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "InetAddress.h"
#include "Socket.h"
#include "Eventloop.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "TcpServer.h"

void test01()
{
    InetAddress listenaddr(8090);
    Socket listenfd(socket(AF_INET, SOCK_STREAM, 0));
    listenfd.bindAddress(listenaddr);
    listenfd.listen();
    for(;;)
    {
        char buff[1024];
        InetAddress peerAddr;
        int connfd = listenfd.accept(&peerAddr);
        printf("conn from %s, port %d\n", 
        inet_ntop(AF_INET, &peerAddr.getSock().sin_addr, buff, sizeof buff), ntohs(peerAddr.getSock().sin_port));
        close(connfd);
    }
}

void newConnection(int sockfd, const InetAddress& peerAddr)     //for test02
{
    InetAddress peerAddr_;
    peerAddr_ = const_cast<InetAddress&>(peerAddr);
    char buff[1024];
    printf("newconn from %s, port %d\n", 
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

void onMessage(const TcpConnectionPtr& conn, char* buf, int size)
{
    printf("onMessage():recv %d bytes from [%s]:%s\n", size, conn->name().c_str(), buf);
}

void onConnection(const TcpConnectionPtr& conn)
{
    if(conn->connected())
    {
        printf("onConnection:new conn [%s]\n", conn->name().c_str());
    }
    else
    {
        printf("onConnection:conn[%s] is down\n", conn->name().c_str());
    }
}

void test03()
{
    InetAddress listenAddr(9898);
    EventLoop loop;
    TcpServer server(&loop, listenAddr, "myserver");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();
    loop.loop();
}

int main()
{
    test03();
    return 0;
}