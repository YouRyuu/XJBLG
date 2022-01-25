#include "SocketsOps.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

typedef struct sockaddr SA;

void setNonblockAndCloseOnExec(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = fcntl(sockfd, F_SETFL, flags);
    flags = fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = fcntl(sockfd, F_SETFD, flags);
}

int createNonblockingOrDie(sa_family_t family)
{
    #ifdef linux
    int sockfd = socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0)
    {
        std::cout<<"error in createNonblockingOrDie"<<std::endl;
        exit(1);
    }
    return sockfd;
    #endif
}

int connect(int sockfd, const struct sockaddr* addr);

void bindOrDie(int sockfd, const struct sockaddr* addr)
{
    int ret = bind(sockfd, addr, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        std::cout<<"error in bindOrDie"<<std::endl;
        exit(1);
    }
}

void listenOrDie(int sockfd)
{
    int ret = listen(sockfd, SOMAXCONN);
    if(ret < 0)
    {
        exit(1);
    }
}

int accept_(int sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    int connfd = accept(sockfd, static_cast<struct sockaddr*>((void*)(addr)), &addrlen);
    if(connfd < 0)
    {

    }
    return connfd;
}

ssize_t read(int sockfd, void* buf, size_t size);
ssize_t write(int sockfd, const void* buf, size_t size);
void close(int sockfd);
void shutdownWrite(int sockfd);
int getSocketError(int sockfd);
bool isSelfConnect(int sockfd);