#ifndef EPOLL_H
#define EPOLL_H

#include <map>
#include <vector>

class Channel;
class EventLoop;
struct pollfd;

class Poller
{
public:
    typedef std::vector<Channel *> ChannelList;
    Poller(EventLoop *loop);
    ~Poller();
    void poll(ChannelList *activeChannels);
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);
    void assertInLoopThread() const;

private:
    void fillActiveChannels(int nunEvents, ChannelList *activeChannels);
    typedef std::vector<struct pollfd> PollfdList;
    typedef std::map<int, Channel *> channelMap;
    channelMap channels_;
    PollfdList pollfds_;
    EventLoop *ownerloop_;
};

struct epoll_event;

class EPoller
{
    //这里使用epoll_event的epoll_data的ptr保存channel
public:
    typedef std::vector<Channel *> ChannelList;
    EPoller(EventLoop *loop);
    ~EPoller();
    void poll(ChannelList *activeChannels);
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);
    void assertInLoopThread() const;
private:
    static const int initEventListSize = 16;
    void fillActiveChannels(int numEvents, ChannelList *activeChannels);
    void update(int op, Channel *channel);
    typedef std::vector<struct epoll_event> EventList;
    int epollfd_;
    EventList events_;
    typedef std::map<int, Channel *> ChannelMap;
    ChannelMap channels_;
    EventLoop *ownerloop_;
};

#endif