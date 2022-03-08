#ifndef WS_CONDITION_H
#define WS_CONDITION_H

#include "Mutex.h"

class Condition
{
    private:
        MutexLock& mutex_;
        pthread_cond_t pcond_;
    public:
        explicit Condition(MutexLock& mutex):mutex_(mutex)
        {
            MCHECK(pthread_cond_init(&pcond_, NULL));
        }

        ~Condition()
        {
            MCHECK(pthread_cond_destroy(&pcond_));
        }

        void wait()
        {
            MutexLock::UnassignGraud ug(mutex_);
            MCHECK(pthread_cond_wait(&pcond_, mutex_.get()));
        }

        void notify()
        {
            MCHECK(pthread_cond_signal(&pcond_));
        }

        void notifyAll()
        {
            MCHECK(pthread_cond_broadcast(&pcond_));
        }

};

#endif