#include "TcpConnection.h"
#include "Channel.h"
#include "Eventloop.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <errno.h>

void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    char buff[1024];
    printf("conn from %s, port %d\n", 
        inet_ntop(AF_INET, &conn->peerAddr().getSock().sin_addr, buff, sizeof buff), ntohs(conn->peerAddr().getSock().sin_port));
}

void defaultMessageCallback(const TcpConnectionPtr&, char*, int)
{
  
}

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
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
//   channel_->setWriteCallback(
//       std::bind(&TcpConnection::handleWrite, this));
//   channel_->setCloseCallback(
//       std::bind(&TcpConnection::handleClose, this));
//   channel_->setErrorCallback(
//       std::bind(&TcpConnection::handleError, this));
  socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
  assert(state_ == Disconnected);
}

const char* TcpConnection::stateToString() const
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

void TcpConnection::startRead()
{
  loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
  //loop_->assertInLoopThread();
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
  //loop_->assertInLoopThread();
  if (reading_ || channel_->isReading())
  {
    channel_->disableReading();
    reading_ = false;
  }
}

void TcpConnection::connectEstablished()
{
  //loop_->assertInLoopThread();
  assert(state_ == Connecting);
  setState(Connected);
  //channel_->tie(shared_from_this());
  channel_->enableReading();
  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
  //loop_->assertInLoopThread();
  if (state_ == Connected)
  {
    setState(Disconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnection::handleRead()
{
  //loop_->assertInLoopThread();
  int savedErrno = 0;
  char buf[1024];
  ssize_t n = read_(channel_->fd(), buf, sizeof(buf));
  if (n > 0)
  {
    messageCallback_(shared_from_this(), buf, n);
  }
  else if (n == 0)
  {
    std::cout<<"read 0"<<std::endl;
  }
  else
  {
    errno = savedErrno;
    std::cout << "TcpConnection::handleRead" <<std::endl;
    //handleError();
  }
}

// void TcpConnection::handleWrite()
// {
//   loop_->assertInLoopThread();
//   if (channel_->isWriting())
//   {
//     ssize_t n = sockets::write(channel_->fd(),
//                                outputBuffer_.peek(),
//                                outputBuffer_.readableBytes());
//     if (n > 0)
//     {
//       outputBuffer_.retrieve(n);
//       if (outputBuffer_.readableBytes() == 0)
//       {
//         channel_->disableWriting();
//         if (writeCompleteCallback_)
//         {
//           loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
//         }
//         if (state_ == kDisconnecting)
//         {
//           shutdownInLoop();
//         }
//       }
//     }
//     else
//     {
//       LOG_SYSERR << "TcpConnection::handleWrite";
//       // if (state_ == kDisconnecting)
//       // {
//       //   shutdownInLoop();
//       // }
//     }
//   }
//   else
//   {
//     LOG_TRACE << "Connection fd = " << channel_->fd()
//               << " is down, no more writing";
//   }
// }

// void TcpConnection::handleClose()
// {
//   loop_->assertInLoopThread();
//   LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
//   assert(state_ == kConnected || state_ == kDisconnecting);
//   // we don't close fd, leave it to dtor, so we can find leaks easily.
//   setState(kDisconnected);
//   channel_->disableAll();

//   TcpConnectionPtr guardThis(shared_from_this());
//   connectionCallback_(guardThis);
//   // must be the last line
//   closeCallback_(guardThis);
// }

// void TcpConnection::handleError()
// {
//   int err = sockets::getSocketError(channel_->fd());
//   LOG_ERROR << "TcpConnection::handleError [" << name_
//             << "] - SO_ERROR = " << err << " " << strerror_tl(err);
// }

