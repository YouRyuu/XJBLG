#include "Eventloop.h"
#include "Channel.h"
#include "Epoll.h"

__thread EventLoop* t_loopInThread = 0;

EventLoop::EventLoop():looping(false), eventHanding(false), quit_(false), poller_(new Poller(this)), currActiveChannel_(nullptr)
{
    if(t_loopInThread)
    {
        abort();
    }
    else
    {
        t_loopInThread = this;
    }
}

EventLoop::~EventLoop()
{
    t_loopInThread = NULL;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThread;
}

void EventLoop::loop()
{
    looping = true;
    while(!quit_)
    {
        activeChannels_.clear();
        poller_->poll(&activeChannels_);
        eventHanding = true;
        for(Channel* channel : activeChannels_)
        {
            currActiveChannel_ = channel;
            currActiveChannel_->handEvent();
        }
        currActiveChannel_ = NULL;
        eventHanding = false;
    }
    looping = false;
}

void EventLoop::runInLoop(Functor cb)
{
    cb();
}

void EventLoop::quit()
{
    quit_ = true;
}

void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}