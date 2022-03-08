#ifndef EVENTLOOPTHREADPOOL
#define EVENTLOOPTHREADPOOL

#include <functional>
#include <memory>
#include <vector>
#include <string>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
    public:
        typedef std::function<void(EventLoop*)> ThreadInitCallback;

        EventLoopThreadPool(EventLoop *baseLoop, const std::string name);
        ~EventLoopThreadPool();
        void setThreadNum(int numThreads)
        {
            numThreads_ = numThreads;
        }
        void start(const ThreadInitCallback &cb = ThreadInitCallback());
        EventLoop* getNextLoop();
        EventLoop* getLoopForHash(size_t hashCode);
        std::vector<EventLoop*> getAllLoops();
        bool started() const { return started_; }
        const std::string &name() const {   return name_; }

    private:
        EventLoop *baseLoop_;   //主线程里的loop
        std::string name_;
        bool started_;
        int numThreads_;
        int next_;
        std::vector<std::unique_ptr<EventLoopThread>> threads_;     //子线程合集
        std::vector<EventLoop*> loops_;     //子loop合集
};

#endif