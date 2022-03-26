#include "Eventloop.h"
#include "Channel.h"
#include "Epoll.h"
#include "Mutex.h"
#include "SocketsOps.h"
#include <sys/eventfd.h>
#include <algorithm>
#include <unistd.h>
#include <iostream>


__thread EventLoop *t_loopInThread = 0;

int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        std::cout << "error in createEventfd()" << std::endl;
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping(false), eventHanding_(false), quit_(false), callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()), wakeupFd_(createEventfd()), poller_(new EPoller(this)),
      currActiveChannel_(nullptr), wakeupChannel_(new Channel(this, wakeupFd_))
{
    if (t_loopInThread)
    {
        std::cout << "error in EventLoop(), a eventloop already exists" << std::endl;
        abort();
    }
    else
    {
        t_loopInThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThread = NULL;
}

EventLoop *EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThread;
}

void EventLoop::loop()
{
    assert(!looping);
    assertInLoopThread();
    looping = true;
    while (!quit_)
    {
        activeChannels_.clear();
        poller_->poll(&activeChannels_);
        eventHanding_ = true;
        for (Channel *channel : activeChannels_)
        {
            currActiveChannel_ = channel;
            currActiveChannel_->handEvent();
        }
        currActiveChannel_ = NULL;
        eventHanding_ = false;
        doPendingFunctors();
    }
    looping = false;
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

size_t EventLoop::queueSize()
{
    MutexLockGuard lock(mutex_);
    return pendingFunctors_.size();
}

void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerloop() == this); //保证是在当前线程执行操作
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    assert(channel->ownerloop() == this);
    assertInLoopThread();
    if (eventHanding_)
    {
        //确保要移除的channel不是活动的
        assert(currActiveChannel_ == channel || std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    assert(channel->ownerloop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    std::cout << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << CurrentThread::tid() << std::endl;
    exit(1);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        std::cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8" << std::endl;
        exit(1);
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        std::cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8" << std::endl;
        exit(1);
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const Functor& func : functors)
    {
        func();
    }
    callingPendingFunctors_ = false;
}