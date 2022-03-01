#ifndef WS_CHANNEL_H
#define WS_CHANNEL_H

#include <functional>
class EventLoop;

class Channel
{
public:
    typedef std::function<void()> EventCallback;
    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handEvent();
    void setReadCallback(EventCallback callback)
    {
        eventCallback = callback;
    }

    int fd()
    {
        return fd_;
    }

    int events()
    {
        return events_;
    }

    void setRevents(int revent)
    {
        revents_ = revent;
    }

    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }

    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }

    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }

    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }

    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }

    bool isNoneEvent()
    {
        return events_==kNoneEvent;
    }

    bool isReading() const {
        return events_ & kReadEvent;
    }

    int index()
    {
        return index_;
    }

    void setIndex(int inx)
    {
        index_ = inx;
    }

    EventLoop* ownerloop()
    {
        return loop_;
    }

    void remove();

     

private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    void update();
    EventLoop* loop_;
    bool eventHanding;
    const int fd_;
    int events_;
    int revents_;
    int index_;
    EventCallback eventCallback;
};

#endif