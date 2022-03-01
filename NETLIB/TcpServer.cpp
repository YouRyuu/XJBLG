#include "TcpServer.h"
#include "Acceptor.h"
#include "Eventloop.h"
#include "SocketsOps.h"

#include <functional>
#include <stdio.h>

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name)
: loop_(loop), 
name_(name), 
acceptor_(new Acceptor(loop, listenAddr, true)), 
connectionCallback_(defaultConnectionCallback),
messageCallback_(defaultMessageCallback),
nextConnId_(1)
{
    int port = ntohs(const_cast<InetAddress&>(listenAddr).getSock().sin_port);
    ipPort = std::to_string(port);
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2)
    );
}

TcpServer:: ~TcpServer()
{
    for(auto& item:connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::start()
{
    assert(!acceptor_->listening());
    loop_->runInLoop(std::bind(&Acceptor::listen, getPointer(acceptor_)));
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    std::cout<< "TcpServer::newConnection [" << name_
           << "] - new connection [" << connName
           << "] from ";
    printf("%d\n",ntohs(const_cast<InetAddress&>(peerAddr).getSock().sin_port));

    InetAddress localAddr(getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(
        loop_, connName, sockfd, localAddr, peerAddr
    ));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    loop_->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    ssize_t n = connections_.erase(conn->name());
    assert(n==1);
    loop_->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}