#include "TcpConnection.h"
#include "Channel.h"
#include "Eventloop.h"
#include "Socket.h"
#include "SocketsOps.h"
#include <stdio.h>
#include <errno.h>

void defaultConnectionCallback(const TcpConnectionPtr &conn)
{
  char buff[1024];
  printf("conn from %s, port %d\n",
         inet_ntop(AF_INET, &conn->peerAddr().getSock().sin_addr, buff, sizeof buff), ntohs(conn->peerAddr().getSock().sin_port));
}

void defaultMessageCallback(const TcpConnectionPtr &, Buffer *buf, int)
{
  buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &nameArg,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(loop),
      name_(nameArg),
      state_(Connecting),
      reading_(true),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr)
{
  channel_->setReadCallback(
      std::bind(&TcpConnection::handleRead, this));
  channel_->setWriteCallback(
      std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(
      std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(
      std::bind(&TcpConnection::handleError, this));
  socket_->setKeepAlive(true);
  socket_->setTcpNoDelay(true);
}

TcpConnection::~TcpConnection()
{
  assert(state_ == Disconnected);
}

const char *TcpConnection::stateToString() const
{
  switch (state_)
  {
  case Disconnected:
    return "Disconnected";
  case Connecting:
    return "Connecting";
  case Connected:
    return "Connected";
  case Disconnecting:
    return "Disconnecting";
  default:
    return "unknown state";
  }
}

void TcpConnection::send(const void *data, int len)
{
  StringPiece message(static_cast<const char *>(data), len);
  send(message);
}

void TcpConnection::send(const StringPiece &data)
{
  if (state_ == Connected)
  {
    if(loop_->isInLoopThread())
    {
      sendInLoop(data);
    }
    else
    {
      void (TcpConnection::*fp)(const StringPiece& data) = &TcpConnection::sendInLoop;
      loop_->runInLoop(std::bind(fp, this, data.as_string()));
    }
  }
}

void TcpConnection::send(Buffer *message)
{
  if (state_ == Connected)
  {
    if(loop_->isInLoopThread())
    {
      //std::cout<<123<<std::endl;
       sendInLoop(message->peek(), message->readableBytes());
      message->retrieveAll();
    }
    else
    {
      //std::cout<<456<<std::endl;
      void (TcpConnection::*fp)(const StringPiece& data) = &TcpConnection::sendInLoop;
      loop_->runInLoop(std::bind(fp, this, message->retrieveAllAsString()));
    }
   
  }
}

void TcpConnection::sendInLoop(const StringPiece &message)
{
  //std::cout<<"****TcpConnection::sendInLoop():"<<message.as_string()<<std::endl;
  sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void *data, size_t len)
{
  loop_->assertInLoopThread();
  //printf("*****%s\n", (const char*)data);
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  if (state_ == Disconnected)
  {
    std::cout << "TcpConnection::sendInLoop() state_ is Disconnected" << std::endl;
  }
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
  {
    // fd??????????????????buf??????????????????????????????????????????socket??????????????????????????????????????????buffer??????
    nwrote = write_(channel_->fd(), data, len); //????????????write????????????
    //std::cout<<"****TcpConnection::sendInLoop(data, len):nwrote:"<<nwrote<<"  size:"<<len<<std::endl;
    if (nwrote >= 0)
    {
      if (nwrote < len) //????????????????????????????????????buffer
      {
        std::cout << "TcpConnection::sendInLoop(): nwrote<len" << std::endl;
      }
    }
    else
    {
      nwrote = 0;
      if (errno != EWOULDBLOCK)
      {
        std::cout << "TcpConnection::sendInLoop(): errno!=EWOULDBLOCK" << std::endl;
      }
    }
  }
  assert(nwrote >= 0);
  if (nwrote < len)
  {
    std::cout<<nwrote<<"is writed, and left"<<len-nwrote<<"not write"<<std::endl;
    outputBuffer_.append(static_cast<const char *>(data) + nwrote, len - nwrote);
    if (!channel_->isWriting())
    {
      channel_->enableWriting(); //????????????socket???????????????????????????handleWrite?????????
    }
  }
}

void TcpConnection::shutdown()
{
  if (state_ == Connected)
  {
    setState(Disconnecting);
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::shutdownInLoop()
{
  loop_->assertInLoopThread();
  if (!channel_->isWriting())
  {
    socket_->shutdownWrite_();
  }
}

void TcpConnection::forceClose()
{
  if (state_ == Connected || state_ == Disconnecting)
  {
    setState(Disconnecting);
    loop_->runInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
  }
}

void TcpConnection::forceCloseInLoop()
{
  loop_->assertInLoopThread();
  if (state_ == Connected || state_ == Disconnecting)
  {
    handleClose();
  }
}

void TcpConnection::startRead()
{
  loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
  loop_->assertInLoopThread();
  if (!reading_ || !channel_->isReading())
  {
    channel_->enableReading();
    reading_ = true;
  }
}

void TcpConnection::stopRead()
{
  loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop()
{
  loop_->assertInLoopThread();
  if (reading_ || channel_->isReading())
  {
    channel_->disableReading();
    reading_ = false;
  }
}

void TcpConnection::connectEstablished()
{
  loop_->assertInLoopThread();
  assert(state_ == Connecting);
  setState(Connected);
  channel_->tie(shared_from_this());
  channel_->enableReading();
  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
  loop_->assertInLoopThread();
  if (state_ == Connected)
  {
    setState(Disconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnection::setTcpNoDelay(bool on)
{
  socket_->setTcpNoDelay(on);
}

void TcpConnection::handleRead()
{
  loop_->assertInLoopThread();
  int savedErrno = 0;
  char buf[1024];
  //ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  ssize_t n = inputBuffer_.bufReadFd(channel_->fd());
  //printf("-----TcpConnection::handleRead()::inputBuffer:%s-----\n", inputBuffer_.getAllStringFromBuffer().c_str());
  if (n > 0)
  {
    messageCallback_(shared_from_this(), &inputBuffer_, n);
  }
  else if (n == 0)
  {
    std::cout<<"TcpConnection::handleRead(): n=0"<<std::endl;
    handleClose();
  }
  else
  {
    errno = savedErrno;
    std::cout << "TcpConnection::handleRead" << std::endl;
    handleError();
  }
}

void TcpConnection::handleClose()
{
  loop_->assertInLoopThread();
  assert(state_ == Connected || state_ == Disconnecting);
  std::cout << "TcpConnection::handleClose" << std::endl;
  setState(Disconnected);
  channel_->disableAll();
  TcpConnectionPtr conn(shared_from_this());    //??????conn????????????????????????????????????
  connectionCallback_(conn); //??????else???????????????????????????
  closeCallback_(conn);
}

void TcpConnection::handleError()
{
  std::cout << "TcpConnection::handleError()" << std::endl;
}

void TcpConnection::handleWrite()
{
  //std::cout<<"TcpConnection::handleWrite"<<std::endl;
  loop_->assertInLoopThread();
  if (channel_->isWriting())
  {
    ssize_t n = write_(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
    if (n > 0)
    {
      //printf("-----TcpConnection::handleWrite()::outputBuffer:%s-----\n", outputBuffer_.getAllStringFromBuffer().c_str());
      outputBuffer_.retrieve(n);
      //std::cout<<n<<" is writen"<<std::endl;
      if (outputBuffer_.readableBytes() == 0)
      {
        //?????????
       // std::cout<<"buffer writen over"<<std::endl;
        channel_->disableWriting();
        if (state_ == Disconnecting)
        {
          shutdownInLoop();
        }
      }
      else
      {
        std::cout << "TcpConnection::handleWrite():going to write" << std::endl;
      }
    }
    else
    {
      std::cout << "TcpConnection::handleWrite():write error" << std::endl;
    }
  }
  else
  {
    //std::cout << "TcpConnection::handleWrite():Connection fd = " << channel_->fd()
     //         << " is down, no more writing" << std::endl;
  }
}