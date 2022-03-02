#ifndef WS_CHANNEL_H
#define WS_CHANNEL_H

#include <functional>
#include <memory>
class EventLoop;

class Channel
{
public:
    typedef std::function<void()> EventCallback;
    Channel(EventLoop* loop, int fd);
    ~Channel();
    void tie(const std::shared_ptr<void>&); // 阻止channel过早析构
    void handEventWithGuard();
    void handEvent();
    void setReadCallback(EventCallback callback)
    {
        readCallback = callback;
    }

    void setWriteCallback(EventCallback cb)
    {
        writeCallback = cb;
    }

    void setCloseCallback(EventCallback cb)
    {
        closeCallback = cb;
    }

    void setErrorCallback(EventCallback cb)
    {
        errorCallback = cb;
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

    bool isWriting() const {
        return events_ & kWriteEvent;
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
    std::weak_ptr<void> tie_;   // 阻止channel过早析构
    bool tied_; // 阻止channel过早析构
    EventCallback readCallback;
    EventCallback writeCallback;
    EventCallback closeCallback;
    EventCallback errorCallback;
};

#endif