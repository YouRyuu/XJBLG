#include "Acceptor.h"
#include "Eventloop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <functional>

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
:loop_(loop), 
acceptSocket_(createBlocking(listenAddr.family())),
acceptChannel_(loop, acceptSocket_.fd()), 
listening_(false), 
idleFd_(open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idleFd_ > 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    close(idleFd_);
}

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >=0)
    {
        if(newCb)
        {
            newCb(connfd, peerAddr);
        }
        else
        {
            close_(connfd);
        }
    }
    else        
    {
        std::cout<<"error in acceptor::handleread"<<std::endl;
        //描述符已满
        if (errno == EMFILE)
        {
            close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
        exit(1);
    }
}