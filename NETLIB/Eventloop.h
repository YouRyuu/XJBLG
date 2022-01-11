#include <sys/types.h>

class EventLoop
{
    public:
    EventLoop();
    ~EventLoop();

    void loop();

    private:
    bool looping;       //是否处于循环
    //const pid_t threadId;    
};