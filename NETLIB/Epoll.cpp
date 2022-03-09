#include "Epoll.h"
#include "Channel.h"
#include <assert.h>
#include <poll.h>
#include "Eventloop.h"
#include <iostream>

Poller::Poller(EventLoop* loop):ownerloop_(loop)
{    
}

Poller::~Poller() = default;

void Poller::assertInLoopThread() const
{
    ownerloop_->assertInLoopThread();
}

void Poller::poll(ChannelList* activeChannels)
{
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), 1000);
    if(numEvents>0)
    {
        fillActiveChannels(numEvents ,activeChannels);
    }
    else if(numEvents==0)
    {
        std::cout<<"Poller::poll() nothing happen"<<std::endl;
    }
    else
    {
        std::cout<<"Poller::poll() error"<<std::endl;
        exit(1);
    }
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels)
{
    for(auto pfd=pollfds_.begin(); pfd!=pollfds_.end() && numEvents > 0; ++pfd)
    {
        if(pfd->revents>0)
        {
            --numEvents;
            auto ch=channels_.find(pfd->fd);
            assert(ch!=channels_.end());
            Channel* channel = ch->second;
            assert(channel->fd()==pfd->fd);
            channel->setRevents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    if(channel->index()<0)    //新来的fd(channel)
    {
        assert(channels_.find(channel->fd())==channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pollfds_.push_back(pfd);
        int index = static_cast<int>(pollfds_.size()-1);
        channel->setIndex(index);
        channels_[pfd.fd] = channel;
    }
    else    //已经存在的fd(channel)
    {
        assert(channels_.find(channel->fd())!=channels_.end());
        assert(channels_[channel->fd()]==channel);
        int index = channel->index();
        assert(index>=0 && index<static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[index];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if(channel->isNoneEvent())      //忽略的fd设置为他的相反数-1
        {
            pfd.fd = -channel->fd()-1;   
        }
    }
}

void Poller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    assert(channels_.find(channel->fd())!=channels_.end());
    assert(channels_[channel->fd()]==channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index>=0 && index<static_cast<int>(pollfds_.size()));
    struct pollfd& pfd = pollfds_[index];
    assert(pfd.fd==-channel->fd()-1 && pfd.events==channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n==1);
    if(index==pollfds_.size()-1)
    {
        //最后一个元素
        pollfds_.pop_back();
    }
    else
    {
        int endChannel = pollfds_.back().fd;    //最后一个元素的fd
        iter_swap(pollfds_.begin()+index, pollfds_.end()-1);    //将当前元素和最后一个元素交换
        if(endChannel<0)
        {
            endChannel = -endChannel - 1;   
        }
        channels_[endChannel]->setIndex(index);     //更新被调换元素的index
        pollfds_.pop_back();
    }

}

bool Poller::hasChannel(Channel* channel)
{
    assertInLoopThread();
    auto it = channels_.find(channel->fd());
    return (it!=channels_.end() && it->second==channel);
}
