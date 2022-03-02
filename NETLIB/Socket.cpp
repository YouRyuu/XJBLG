#include "Socket.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <iostream>

Socket::~Socket()
{
    close_(sockfd_);
}

void Socket::bindAddress(const InetAddress& localAddr)
{
    bindOrDie(sockfd_, localAddr.getSockAddr());
}

void Socket::listen()
{
    listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peerAddr)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    int connfd = accept_(sockfd_, &addr);
    if(connfd > 0)
    {
        peerAddr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::shutdownWrite_()
{
    shutdownWrite(sockfd_);
}

void Socket::setReuseAddr(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
  int optval = on ? 1 : 0;
  int ret = setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on)
  {
    std::cout << "Socket::setReusePort:SO_REUSEPORT failed." << std::endl;
    exit(1);
  }
#else
  if (on)
  {
    std::cout << "Socket::setReusePort:SO_REUSEPORT is not supported." << std::endl;
    exit(1);
  }
#endif
}

void Socket::setKeepAlive(bool on)
{
  int optval = on ? 1 : 0;
  setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}