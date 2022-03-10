#include "TcpServer.h"
#include "Acceptor.h"
#include "Eventloop.h"
#include "SocketsOps.h"
#include "EventLoopThreadPool.h"
#include <functional>
#include <stdio.h>

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name)
: loop_(loop), 
name_(name), 
acceptor_(new Acceptor(loop, listenAddr, true)), threadPool_(new EventLoopThreadPool(loop, name)), 
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
    loop_->assertInLoopThread();
    for(auto& item:connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int num)
{
    assert(num>=0);
    threadPool_->setThreadNum(num);
}

void TcpServer::start()
{
    threadPool_->start(threadInitCallback_);
    assert(!acceptor_->listening());
    loop_->runInLoop(std::bind(&Acceptor::listen, getPointer(acceptor_)));
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    loop_->assertInLoopThread();
    EventLoop* ioLoop = threadPool_->getNextLoop();
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
        ioLoop, connName, sockfd, localAddr, peerAddr
    ));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));     //将新的conn给一个新的线程，调用runInLoop，
                                                                                //因为这时候是在主线程里面调用的runInLoop，而这个ioLoop是属于新线程的，所以会调用queueLoop
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    ssize_t n = connections_.erase(conn->name());
    assert(n==1);
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}