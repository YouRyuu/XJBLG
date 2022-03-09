#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include <deque>
#include <vector>

class ThreadPool
{
    public:
        typedef std::function<void()> Task;
        explicit ThreadPool(const std::string &name=std::string("threadpool"));
        ~ThreadPool();

        void setMaxQueueSize(int maxSize)
        {
            maxQueueSize_ = maxSize;
        }

        void setThreadInitCallback(const Task& cb)
        {
            threadInitCallback_ = cb;
        }

        void start(int numThreads);
        void stop();

        const std::string& name() const
        {
            return name_;
        }

        size_t queueSize();

        void run(Task task);

    private:
        bool isFull();
        void runInThread();
        Task take();
        MutexLock mutex_;
        Condition notEmpty_;
        Condition notFull_;
        std::string name_;
        Task threadInitCallback_;
        std::vector<std::unique_ptr<Thread> > threads_;
        std::deque<Task> queue_;
        size_t maxQueueSize_;
        bool running_;
};

#endif