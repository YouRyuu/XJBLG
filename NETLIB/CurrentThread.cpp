#include "CurrentThread.h"
#include <stdlib.h>

namespace CurrentThread
{
    __thread int t_cacheTid = 0;           //线程id
    __thread char t_tidString[32];     //线程id
    __thread int t_tidStringLength = 6;    // char的大小
    __thread const char *t_threadName = "unknown"; //线程名称
}