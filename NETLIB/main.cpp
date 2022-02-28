#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "InetAddress.h"
#include "Socket.h"

int main()
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
    return 0;
}