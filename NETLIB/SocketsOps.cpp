#include "SocketsOps.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/uio.h>        //readv writev
#include <iostream>

typedef struct sockaddr SA;

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const struct sockaddr*>((const void*)addr);
}

const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr)
{
    return static_cast<const struct sockaddr_in*>((const void*)addr);
}

struct sockaddr* sockaddr_cast(struct sockaddr_in* addr)
{
    return static_cast<struct sockaddr*>((void*)addr);
}

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
        std::cout<<"SocketsOps:error in createNonblockingOrDie"<<std::endl;
        exit(1);
    }
    return sockfd;
    #else
    int sockfd = socket(family, SOCK_STREAM, 0);
    setNonblockAndCloseOnExec(sockfd);
    return sockfd;
    #endif
}

int createBlocking(sa_family_t family)
{
    int sockfd = socket(family,SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        std::cout<<"SocketsOps:error in create Blocking"<<std::endl;
        exit(1);
    }
    return sockfd;
}

int connect_(int sockfd, const struct sockaddr* addr)
{
    int connfd = connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    return connfd;
}

void bindOrDie(int sockfd, const struct sockaddr* addr)
{
    int ret = bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    if (ret < 0)
    {
        std::cout<<"SocketsOps:error in bindOrDie"<<std::endl;
        exit(1);
    }
}

void listenOrDie(int sockfd)
{
    int ret = listen(sockfd, SOMAXCONN);
    if(ret < 0)
    {
        std::cout<<"SocketsOps:error in listenOrDie"<<std::endl;
        exit(1);
    }
}

int accept_(int sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    int connfd = accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonblockAndCloseOnExec(connfd);
    if(connfd < 0)
    {
        int savedErrno = errno;
        switch (savedErrno)
    {   //  哪些错误是预料之中的
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:
      case EPERM:
      case EMFILE:
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        std::cout << "SocketsOps:unexpected error of ::accept " << std::endl;
        exit(1);
        break;
      default:
        std::cout << "SocketsOps:unknown error of ::accept " << std::endl;
        exit(1);
        break;
    }
    }
    return connfd;
}

ssize_t read_(int sockfd, void* buf, size_t size)
{
    return read(sockfd, buf, size);
}
ssize_t write_(int sockfd, const void* buf, size_t size)
{
    return write(sockfd, buf, size);
}

ssize_t readv_(int sockfd, const struct iovec* iov, int iovcount)
{
    return readv(sockfd, iov, iovcount);
}

void close_(int sockfd)
{
    int ret = close(sockfd);
    if(ret<0)
    {
        std::cout<<"SocketsOps:error in close"<<std::endl;
    }
}

void shutdownWrite(int sockfd)
{
    if(shutdown(sockfd, SHUT_WR) < 0)
    {
        std::cout<<"SocketsOps:error in shutdown"<<std::endl;
    }
}

int getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

bool isSelfConnect(int sockfd)
{
    return false;
}

struct sockaddr_in getLocalAddr(int sockfd)
{
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if(getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen)<0)
    {
        std::cout<<"SocketsOps:socketsops getlocalAddr error"<<std::endl;
        exit(1);
    }
    return localaddr;
}

struct sockaddr_in getPeerAddr(int sockfd)
{
    struct sockaddr_in peeraddr;
    memset(&peeraddr, 0, sizeof(peeraddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if(getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen)<0)
    {
        std::cout<<"SocketsOps:socketsops getpeerAddr error"<<std::endl;
        exit(1);
    }
    return peeraddr;
}