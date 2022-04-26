#include "Epoll.h"
#include "Channel.h"
#include "Eventloop.h"
#include <iostream>
#include <string.h>
#include <assert.h>
#include <poll.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

Poller::Poller(EventLoop *loop) : ownerloop_(loop)
{
}

Poller::~Poller() = default;

void Poller::assertInLoopThread() const
{
    ownerloop_->assertInLoopThread();
}

void Poller::poll(ChannelList *activeChannels)
{

    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), 1000);
    if (numEvents > 0)
    {
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (numEvents == 0)
    {
        // std::cout<<"Poller::poll() nothing happen"<<std::endl;
    }
    else
    {
        std::cout << "Poller::poll() error" << std::endl;
        exit(1);
    }
}

void Poller::fillActiveChannels(int numEvents, ChannelList *activeChannels)
{
    for (auto pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --numEvents;
            auto ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel *channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->setRevents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel *channel)
{
    assertInLoopThread();
    if (channel->index() < 0) //新来的fd(channel)
    {
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pollfds_.push_back(pfd);
        int index = static_cast<int>(pollfds_.size() - 1);
        channel->setIndex(index);
        channels_[pfd.fd] = channel;
    }
    else //已经存在的fd(channel)
    {
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int index = channel->index();
        assert(index >= 0 && index < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[index];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) //忽略的fd设置为他的相反数-1
        {
            pfd.fd = -channel->fd() - 1;
        }
    }
}

void Poller::removeChannel(Channel *channel)
{
    assertInLoopThread();
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index >= 0 && index < static_cast<int>(pollfds_.size()));
    struct pollfd &pfd = pollfds_[index];
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1);
    if (index == pollfds_.size() - 1)
    {
        //最后一个元素
        pollfds_.pop_back();
    }
    else
    {
        int endChannel = pollfds_.back().fd;                     //最后一个元素的fd
        iter_swap(pollfds_.begin() + index, pollfds_.end() - 1); //将当前元素和最后一个元素交换
        if (endChannel < 0)
        {
            endChannel = -endChannel - 1;
        }
        channels_[endChannel]->setIndex(index); //更新被调换元素的index
        pollfds_.pop_back();
    }
}

bool Poller::hasChannel(Channel *channel)
{
    assertInLoopThread();
    auto it = channels_.find(channel->fd());
    return (it != channels_.end() && it->second == channel);
}

const int New = -1;
const int Added = 2;
const int Deleted = 3;

EPoller::EPoller(EventLoop *loop)
    : ownerloop_(loop), epollfd_(epoll_create1(EPOLL_CLOEXEC)), events_(initEventListSize)
{
    if (epollfd_ < 0)
    {
        std::cout << "EPoller::EPoller() epollfd_ < 0"<< std::endl;
        exit(1);
    }
}

EPoller::~EPoller()
{
    close(epollfd_);
}
void EPoller::poll(ChannelList *activeChannels)
{
    int numEvents = epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), 1000);
    if (numEvents > 0)
    {
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        //std::cout<<"epoll numEvents ret 0"<<std::endl;
    }
    else
    {
        std::cout << "EPoller::poll(): error in numEvents < 0" << std::endl;
        exit(1);
    }
}
void EPoller::updateChannel(Channel *channel)
{
    assertInLoopThread();
    const int index = channel->index();
    if (index == New || index == Deleted)
    {
        int fd = channel->fd();
        if (index == New) //  新来的
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->setIndex(Added);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        //更新已经存在的
        int fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == Added);
        if (channel->isNoneEvent())
        {
            //删除
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(Deleted);
        }
        else
        {
            //更新
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPoller::removeChannel(Channel *channel)
{
    assertInLoopThread();
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == Added || index == Deleted);
    size_t n = channels_.erase(fd);
    assert(n == 1);
    if (index == Added)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(New);
}

bool EPoller::hasChannel(Channel *channel)
{
    assertInLoopThread();
    auto it = channels_.find(channel->fd());
    return (it != channels_.end() && it->second == channel);
}

void EPoller::assertInLoopThread() const
{
    ownerloop_->assertInLoopThread();
}

void EPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels)
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        int fd = channel->fd();
        auto it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
        channel->setRevents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPoller::update(int op, Channel *channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (epoll_ctl(epollfd_, op, fd, &event) < 0)
    {
        std::cout << "EPoller::update:error in fd:" << fd << std::endl;
        exit(0);
    }
}
