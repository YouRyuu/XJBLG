#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "InetAddress.h"
#include "Socket.h"
#include "Eventloop.h"
#include "Acceptor.h"

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

void newConnection(int sockfd, const InetAddress& peerAddr)
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

int main()
{
    test02();
    return 0;
}