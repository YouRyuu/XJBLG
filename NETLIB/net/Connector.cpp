#include "Connector.h"
#include "Channel.h"
#include "Eventloop.h"
#include "SocketsOps.h"
#include <errno.h>
#include <assert.h>

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect__(false),
      state_(Disconnected),
      retryDelayMs_(kInitRetryDelayMs)
{
}

Connector::~Connector()
{
    assert(!channel_);
}

void Connector::start()
{
    connect__ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this)); // FIXME: unsafe
}

void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    assert(state_ == Disconnected);
    if (connect__)
    {
        connect(); //尝试进行连接
    }
    else
    {
        printf("do not connect\n");
    }
}

void Connector::stop()
{
    connect__ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == Connecting)
    {
        setState(Disconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    //连接成功之后sockfd就可写了
    int sockfd = createNonblockingOrDie(serverAddr_.family());
    int ret = connect_(sockfd, serverAddr_.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;
    //printf("in connector::connect:%d,%d,%d\n", ret, sockfd, savedErrno);
    switch (savedErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd); //连接成功
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        printf("connect error in Connector::startInLoop %d\n", savedErrno);
        close_(sockfd);
        break;

    default:
        printf("Unexpected error in Connector::startInLoop ");
        close_(sockfd);
        // connectErrorCallback_();
        break;
    }
}

void Connector::restart() //重连
{
    loop_->assertInLoopThread();
    setState(Disconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect__ = true;
    startInLoop();
}

void Connector::connecting(int sockfd) //这时候需要判断是连接成功了还是有错误
{
    setState(Connecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(
        std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
    channel_->setErrorCallback(
        std::bind(&Connector::handleError, this)); // FIXME: unsafe

    // channel_->tie(shared_from_this()); is not working,
    // as channel_ is not managed by shared_ptr
    channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}

void Connector::handleWrite()
{
    // printf("Connector::handleWrite ");
    if (state_ == Connecting)
    {
        int sockfd = removeAndResetChannel();
        int err = getSocketError(sockfd);
        if (err) //发生了错误
        {
            printf("Connector::handleWrite - SO_ERROR");
            retry(sockfd);
        }
        else if (isSelfConnect(sockfd)) //自连接
        {
            printf("Connector::handleWrite - Self connect");
            retry(sockfd);
        }
        else //连接成功
        {
            setState(Connected);
            if (connect_)
            {
                //连接成功之后立即回调connection callback
                newConnectionCallback_(sockfd);
            }
            else
            {
                close_(sockfd);
            }
        }
    }
    else
    {
        assert(state_ == Disconnected);
    }
}

void Connector::handleError()
{
    if (state_ == Connecting)
    {
        int sockfd = removeAndResetChannel();
        int err = getSocketError(sockfd);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    close_(sockfd);
    setState(Disconnected);
    if (connect__)
    {
        loop_->runInLoop(std::bind(&Connector::startInLoop, shared_from_this()));
    }
    else
    {
        printf("do not connect");
    }
}
