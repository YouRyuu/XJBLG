#include "Channel.h"
#include "Eventloop.h"
#include <poll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd):
loop_(loop), eventHanding(false),fd_(fd), events_(0), revents_(0), index_(-1)
{
}

Channel::~Channel()
{
    if(loop_->hasChannel(this))
    {
        remove();
    }
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::handEvent()
{
    eventHanding = true;
    if(revents_ & (POLLIN | POLLPRI))
    {
        if(eventCallback)
        {
            eventCallback();
        }
    }
    eventHanding = false;
}