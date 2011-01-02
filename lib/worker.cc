//
// worker.cc
//

#include <smache/worker.hh>

Worker::Worker(WorkPool* workpool) : running(true), pool(workpool)
{
    start();
}

void Worker::run()
{
    while( this->running )
    {
        Task* t = this->pool->next();
        t->execute();
        delete t;
    }
}

Worker::~Worker()
{
    this->running = false;
}

WorkPool::WorkPool()
{
}

WorkPool::~WorkPool()
{
    this->acquire();
    while( this->workers.size() > 0 ) {
        Worker* w = this->workers.back();
        this->workers.pop_back();
        delete w;
    }
    this->release();
}
   
Task* WorkPool::next()
{
    this->acquire();
    while( this->tasks.size() == 0 ) {
        this->available++;
        this->wait();
        this->available--;
    }
    Task* t = this->tasks.front();
    this->tasks.pop_front(); 
    this->release();
    return t;
}

void WorkPool::submit(Task* t)
{
    this->acquire();
    this->tasks.push_back(t);
    this->broadcast();
    if( this->available == 0 ) {
        Worker* w = new Worker(this);
        this->workers.push_back(w);
    }
    this->release();
}
