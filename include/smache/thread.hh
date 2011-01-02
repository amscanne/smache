//
// thread.hh
//

#ifndef _THREAD_HH_
#define _THREAD_HH_

#include <pthread.h>
#include <signal.h>

class Thread;

static inline void* bounce(void* arg);

class Thread {
private:
    pthread_t tid;
public:
    Thread() : tid(-1)
    {
    }
    ~Thread()
    {
        stop();
    }
    virtual void start()
    {
        if( this->tid == (pthread_t)-1 ) 
        {
            pthread_attr_t tattr;
            pthread_attr_init(&tattr);
            pthread_create(&(this->tid), &tattr, bounce, this);
        }
    }
    virtual void stop()
    {
        if( this->tid != (pthread_t)-1 ) 
        {
            pthread_kill(this->tid, SIGKILL);
            this->tid = -1;
        }
    }
    virtual void restart()
    {
        stop();
        start();
    }
    virtual void run() = 0;
};

static inline void* bounce(void* arg)
{
    ((Thread*)(arg))->run();
    return arg;
}

#endif // _THREAD_HH_
