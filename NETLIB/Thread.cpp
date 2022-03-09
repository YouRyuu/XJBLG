#include "Thread.h"
#include "CurrentThread.h"
#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <exception>
//#include <sys/prctl.h>

pid_t gettid()
{
    return static_cast<pid_t>(syscall(SYS_gettid));     //获取线程的pid，不同于pthread_self获得的，这个是可以在top里面看到的
}

namespace CurrentThread
{

    void cacheTid()
    {
        if (t_cacheTid == 0)
        {
            t_cacheTid = gettid();
            t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cacheTid);
        }
    }

    bool isMainThread()
    {
        return tid() == getpid();       //通过进程pid和每个线程的pid判断是不是主线程
    }
}

class ThreadNameInitializer
{
 public:
  ThreadNameInitializer()
  {
    CurrentThread::t_threadName = "main";
    CurrentThread::tid();
  }
};

ThreadNameInitializer init;

class ThreadData
{
public:
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t *tid_;
    CountDownLatch *latch_;

    ThreadData(ThreadFunc func, const std::string name, pid_t *tid, CountDownLatch *latch)
        : func_(std::move(func)), name_(name), tid_(tid), latch_(latch)
    {
    }

    void runInThread()
    {
        *tid_ = CurrentThread::tid();
        tid_ = NULL; //因为这里tid_是一个指针，指向Thread里的，为了防止后续把他获取到的值意外改了，所以又置为NULL
        latch_->countDown();
        latch_ = NULL; //
        CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
        // prctl(PR_SET_NAME, CurrentThread::t_threadName);      //in linux

        try
        {
            func_();
            CurrentThread::t_threadName = "finished";
        }
        catch (const std::exception &ex)
        {
            CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...)
        {
            CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
            throw; // rethrow
        }
    }
};

void *startThread(void *obj)
{
    ThreadData *data = static_cast<ThreadData *>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false), joined_(false), pthreadId_(0), tid_(0), func_(std::move(func)), name_(name), latch_(1)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName()
{
    if (name_.empty())
    {
        char buf[10];
        snprintf(buf, sizeof buf, "Thread");
        name_ = buf;
    }
}

void Thread::start()
{
    assert(!started_);
    started_ = true;
    ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
    if(pthread_create(&pthreadId_, NULL, &startThread, data))
    {
        started_ = false;
        delete data;
        std::cout<<"Thread::start(): error in pthread_create"<<std::endl;
        exit(1);
    }
    else
    {
        latch_.wait();  //主线程等待，确保新线程执行
        assert(tid_ > 0);
    }
}

int Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}