// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "TcpClient.h"
#include "Connector.h"
#include "Eventloop.h"
#include "SocketsOps.h"

#include <stdio.h>

namespace func
{
  void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn)
  {
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  }

  void removeConnector(const ConnectorPtr &connector)
  {
    // connector->
  }
}

TcpClient::TcpClient(EventLoop *loop,
                     const InetAddress &serverAddr,
                     const std::string &nameArg)
    : loop_(loop),
      connector_(new Connector(loop, serverAddr)),
      name_(nameArg),
      connectionCallback_(defaultConnectionCallback),   //这个是给TcpConnection用的
      messageCallback_(defaultMessageCallback),
      retry_(false),
      connect_(true),
      nextConnId_(1)
{
  connector_->setNewConnectionCallback(
      std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

TcpClient::~TcpClient() // client析构，关闭连接
{
  TcpConnectionPtr conn;
  bool unique = false;
  {
    MutexLockGuard lock(mutex_);
    unique = connection_.unique(); //记录这个connection引用计数是不是为1
    conn = connection_;
  }
  if (conn)
  {
    assert(loop_ == conn->getLoop());
    // FIXME: not 100% safe, if we are in different thread
    CloseCallback cb = std::bind(&func::removeConnection, loop_, std::placeholders::_1);
    loop_->runInLoop(
        std::bind(&TcpConnection::setCloseCallback, conn, cb));
    if (unique)   //如果没有别的引用了
    {
      conn->forceClose();
    }
  }
  else
  {
    connector_->stop();
    // FIXME: HACK
    //loop_->runInLoop(std::bind(&func::removeConnector, connector_));
  }
}

void TcpClient::connect()
{
  // FIXME: check state
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect()
{
  connect_ = false;

  {
    MutexLockGuard lock(mutex_);
    if (connection_)
    {
      connection_->shutdown();
    }
  }
}

void TcpClient::stop()
{
  connect_ = false;
  connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
  loop_->assertInLoopThread();
  InetAddress peerAddr(getPeerAddr(sockfd)); //获取对端的信息
  int port = ntohs(const_cast<InetAddress &>(peerAddr).getSock().sin_port);
  std::string ipPort = std::to_string(port);
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", ipPort.c_str(), nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;

  InetAddress localAddr(getLocalAddr(sockfd)); //本端的信息
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(loop_,
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr));
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  // conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpClient::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe
  {
    MutexLockGuard lock(mutex_);
    connection_ = conn;
  }
  conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn)
{
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());
  {
    MutexLockGuard lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }
  loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  if (retry_ && connect_)
  {
    connector_->restart();
  }
}
