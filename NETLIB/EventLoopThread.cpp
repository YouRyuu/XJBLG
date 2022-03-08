#include "EventLoopThread.h"
#include "Eventloop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
:loop_(NULL), exiting_(false), thread_(std::bind(&EventLoopThread::threadFunc, this), name), mutex_(), cond_(mutex_), callback_(cb)
{   
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_!=NULL)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_.started());
    thread_.start();        //调用start后会建立新线程，此时主线程返回到这里继续执行，阻塞在下面的cond_.wait()那里
                            //等到子线程执行绑定的函数，也就是下面的threadFunc，新建了子loop，然后cond_.notify()
                            //主线程就获得了子线程的loop
    EventLoop *loop = NULL;
    {
        MutexLockGuard lock(mutex_);
        while(loop_==NULL)
        {
            cond_.wait();
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;     //子线程的loop
    if(callback_)
    {
        callback_(&loop);
    }
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }
    loop.loop();

    MutexLockGuard lock(mutex_);
    loop_ = NULL;
}