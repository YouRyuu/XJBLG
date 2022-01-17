#include "Eventloop.h"
#include <map>

class Channel;

class Poller
{
public:
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);
    typedef std::vector<Channel*> ChannelList;
    Poller(EventLoop* loop);
    void poll(ChannelList* activeChannels);
private:
    void fillActiveChannels(int nunEvents, ChannelList* activeChannels);
    typedef std::vector<struct pollfd> PollfdList;
    typedef std::map<int, Channel*> channelMap;
    channelMap channels_;
    EventLoop* ownerloop_;
    PollfdList pollfds_;
};