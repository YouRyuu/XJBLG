#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include "Mutex.h"
#include "Condition.h"

class CountDownLatch
{
    /*
    countdowklatch的作用在于保证创建的线程的func运行之后，调用方的start函数再返回
    */
    private:
        MutexLock mutex_;
        Condition condition_;
        int count_;
    public:
        explicit CountDownLatch(int count);
        void wait();
        void countDown();
        int getCount();
};

#endif