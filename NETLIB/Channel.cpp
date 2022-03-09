#include "Channel.h"
#include "Eventloop.h"
#include <poll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd):
loop_(loop), eventHanding(false),fd_(fd), events_(0), revents_(0), index_(-1), tied_(false), addedToLoop_(false)
{
}

Channel::~Channel()
{
    assert(!eventHanding);
    assert(!addedToLoop_);
    if(loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::update()
{
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove()
{
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
    //将channel的拥有者绑定，防止channel被拥有者（如TcpConnection）析构
    tie_ = obj;
    tied_ = true;
}

void Channel::handEvent()
{
    std::shared_ptr<void> guard;
    if(tied_)
    {
        guard = tie_.lock();
        if(guard)
        {
            handEventWithGuard();
        }
    }
    else
    {
        handEventWithGuard();
    }
}

void Channel::handEventWithGuard()
{
    eventHanding = true;
    //outputEvent(revents_);
    if((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        if(closeCallback)
        {
            closeCallback();
        }    
    }

    if(revents_ & POLLNVAL)
    {
        std::cout <<"Channel:handEvent(): POLLNVAL"<<std::endl;
    }

    if(revents_ & (POLLERR | POLLNVAL))
    {
        if(errorCallback)
        {
            errorCallback();
        }
    }

    if(revents_ & (POLLIN | POLLPRI))
    {
        if(readCallback)
        {
            readCallback();
        }
    }

    if(revents_ & POLLOUT)
    {
        if(writeCallback)
        {
            writeCallback();
        }
    }
    eventHanding = false;
}

void Channel::outputEvent(int event)
{
    if(event & POLLIN)
    {
        std::cout<<"POLLIN;"<<std::endl;
    }
    if(event & POLLPRI)
    {
        std::cout<<"POLLPRI;"<<std::endl;
    }
    if(event & POLLOUT)
    {
        std::cout<<"POLLOUT;"<<std::endl;
    }
    if(event & POLLHUP)
    {
        std::cout<<"POLLHUP;"<<std::endl;
    }
    if(event & POLLERR)
    {
        std::cout<<"POLLERR;"<<std::endl;
    }
    if(event & POLLNVAL)
    {
        std::cout<<"POLLNVAL;"<<std::endl;
    }
}