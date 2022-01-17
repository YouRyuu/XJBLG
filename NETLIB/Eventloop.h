#include <sys/types.h>
#include <pthread.h>
#include <iostream>

class EventLoop
{
    public:
    EventLoop();
    ~EventLoop();

    void loop();
    static EventLoop* getEventLoopOfCurrentThread();

    private:
    bool looping;       //是否处于循环
    //const pid_t threadId;    
};