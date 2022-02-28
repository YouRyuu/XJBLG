#ifndef WS_EPOLL_H
#define WS_EPOLL_H

#include <map>
#include <vector>


class Channel;
class EventLoop;
struct pollfd;

class Poller
{
public:
    typedef std::vector<Channel*> ChannelList;
    Poller(EventLoop* loop);
    ~Poller();
    void poll(ChannelList* activeChannels);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);
private:
    void fillActiveChannels(int nunEvents, ChannelList* activeChannels);
    typedef std::vector<struct pollfd> PollfdList;
    typedef std::map<int, Channel*> channelMap;
    channelMap channels_;
    PollfdList pollfds_;
    EventLoop* ownerloop_;
};

#endif