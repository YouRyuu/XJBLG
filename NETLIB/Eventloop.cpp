#include "Eventloop.h"

__thread EventLoop* t_loopInThread = 0;

EventLoop::EventLoop():looping(false){}

EventLoop::~EventLoop()
{
    t_loopInThread = NULL;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThread;
}

void EventLoop::loop()
{
    //looping = true;
}