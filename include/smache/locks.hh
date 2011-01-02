//
// locks.hh
//

#ifndef _LOCKS_HH_
#define _LOCKS_HH_

#include <stdint.h>
#include <pthread.h>

#include <smache/log.hh>

class Safe;

class Safe {
private:
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    uint32_t        waiters;

protected:
    Safe()
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mutex, &attr);
        pthread_cond_init(&cond, NULL);
        waiters = 0;
    }
    ~Safe()
    {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }
    void acquire()
    {
        SMACHE_DEBUG(LEVEL_DEBUG, "Acquiring %p...\n", this);
        pthread_mutex_lock(&mutex);
        SMACHE_DEBUG(LEVEL_DEBUG, "Acquired %p.\n", this);
    }
    void release()
    {
        SMACHE_DEBUG(LEVEL_DEBUG, "Releasing %p...\n", this);
        pthread_mutex_unlock(&mutex);
        SMACHE_DEBUG(LEVEL_DEBUG, "Released %p.\n", this);
    }
    void wait()
    {
        waiters++;
        pthread_cond_wait(&cond, &mutex);
        waiters--;
    }
    void signal()
    {
        if( waiters > 0 ) {
            pthread_cond_signal(&cond);
        }
    }
    void broadcast()
    {
        if( waiters > 0 ) {
            pthread_cond_broadcast(&cond);
        }
    }
};

#endif // _LOCKS_HH_
