#include <sys/types.h>
#include <pthread.h>
#include <iostream>
#include <vector>

class Channel;
class Poller;

class EventLoop
{
public:
    EventLoop();
    ~EventLoop();

    void loop();
    static EventLoop* getEventLoopOfCurrentThread();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);
    void quit();

private:
    typedef std::vector<Channel *>ChannelList;
    bool looping;       //是否处于循环
    bool eventHanding;
    bool quit_;
    //const pid_t threadId;  
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;
    Channel* currActiveChannel_;  
};