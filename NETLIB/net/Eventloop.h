#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "CurrentThread.h"
#include "Mutex.h"
#include "TimeQueue.h"
#include "TimerID.h"
#include <sys/types.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <functional>
#include <memory>

class Channel;
class Poller;
class EPoller;

class EventLoop
{
public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();

    void loop();
    static EventLoop *getEventLoopOfCurrentThread();
    void wakeup();
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);
    void quit();
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
    size_t queueSize();
    //定时器相关
    TimerId runAt(Timestamp time, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancel(TimerId timerId);

    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const
    {
        return threadId_ == CurrentThread::tid();
    }

    bool eventHandling() const
    {
        return eventHanding_;
    }
    const pid_t threadId_;

private:
    void abortNotInLoopThread();
    void handleRead(); // for wake up
    void doPendingFunctors();

    typedef std::vector<Channel *> ChannelList;

    bool looping; //是否处于循环
    bool eventHanding_;
    bool quit_;
    bool callingPendingFunctors_;

    int wakeupFd_;
    std::unique_ptr<EPoller> poller_;
    ChannelList activeChannels_;
    Channel *currActiveChannel_;
    std::vector<Functor> pendingFunctors_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::unique_ptr<TimerQueue> timerQueue_;
    MutexLock mutex_;
};

#endif