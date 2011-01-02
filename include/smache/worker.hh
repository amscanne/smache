//
// worker.hh
//

#ifndef _WORKER_HH_
#define _WORKER_HH_

#include <list>

class Task;
class Worker;
class WorkPool;

#include <smache/locks.hh>
#include <smache/thread.hh>

class Task {
public:
    virtual void execute();
};

class Worker : protected Thread {
protected:
    bool running;

public:
    WorkPool* pool;
    Worker(WorkPool* workpool);
    ~Worker();

    virtual void run();
};

class WorkPool : protected Safe {
protected:
    uint32_t available;
    std::list<Worker*> workers;
    std::list<Task*> tasks;

public:
    WorkPool();
    ~WorkPool();

    Task* next();
    void submit(Task* t);
};

#endif // _WORKER_HH_
