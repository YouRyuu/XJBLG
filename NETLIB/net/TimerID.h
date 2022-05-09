#ifndef TIMERID_H
#define TIMERID_H

class Timer;

class TimerId
{
public:
    TimerId()
        : timer_(nullptr),
          sequence_(0)
    {
    }

    TimerId(Timer *timer, int seq)
        : timer_(timer),
          sequence_(seq)
    {
    }

    friend class TimerQueue;

private:
    Timer *timer_;
    int sequence_;
};
#endif