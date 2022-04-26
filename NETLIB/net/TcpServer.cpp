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
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));        //这里的loop_指的是主线程，但是调用它的可能是某个子线程
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    ssize_t n = connections_.erase(conn->name());
    assert(n==1);
    EventLoop* ioLoop = conn->getLoop();            //得到所属的loop，可能不是主线程
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));     //bind可以延长conn的生命期到connectDestroy，防止过早析构
                                                                                //同时用queueInLoop也可以防止过早析构
    //ioLoop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));     //bind可以延长conn的生命期到connectDestroy，防止过早析构
}