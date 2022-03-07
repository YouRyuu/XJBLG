#ifndef WS_MUTEX_H
#define WS_MUTEX_H

#include "CurrentThread.h"
#include <assert.h>
#include <pthread.h>


//这个宏是用来检查返回值的，__typeof__就是取类型
#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

class MutexLock
{
    public:
        MutexLock()
        {
            MCHECK(pthread_mutex_init(&mutex_, NULL));
        }

        ~MutexLock()
        {
            MCHECK(pthread_mutex_destroy(&mutex_));
        }

        void lock()
        {
            MCHECK(pthread_mutex_lock(&mutex_));
        }

        void unlock()
        {
            MCHECK(pthread_mutex_unlock(&mutex_));
        }

        pthread_mutex_t* get()
        {
            return &mutex_;
        }

    private:
        pthread_mutex_t mutex_;
        friend class Condition;
};

class MutexLockGuard
{
    private:
        MutexLock &mutex_;
    public:
        explicit MutexLockGuard(MutexLock &mutex):mutex_(mutex)
        {
            mutex_.lock();
        }

        ~MutexLockGuard()
        {
            mutex_.unlock();
        }
};

#endif