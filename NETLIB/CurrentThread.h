#ifndef WS_CURRENTTHREAD_H
#define WS_CURRENTTHREAD_H

#include <string>
#include <unistd.h>
#include <sys/syscall.h> //@ syscall
#include <sys/types.h>

namespace CurrentThread
{
    extern __thread int t_cacheTid;           //线程id
    extern __thread char t_tidString[32];     //线程id
    extern __thread int t_tidStringLength;    // char的大小
    extern __thread const char *t_threadName; //线程名称
    void cacheTid();

    inline int tid()
    {
        if (t_cacheTid == 0)
        {
            cacheTid();
        }
        return t_cacheTid;
    }

    inline const char *tidString() // for logging
    {
        return t_tidString;
    }

    inline int tidStringLength() // for logging
    {
        return t_tidStringLength;
    }

    inline const char *name()
    {
        return t_threadName;
    }

    bool isMainThread();
}

#endif
