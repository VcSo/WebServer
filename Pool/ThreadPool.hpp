#ifndef ThreadPool_H
#define ThreadPool_H

#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <queue>

#include "../Lock/Locker.h"

template <typename T>
class ThreadPool
{
public:
    ThreadPool(int actor, int threadnum, int max_request = 10000);
    ~ThreadPool();

    bool append();
    bool append_p();


private:
    static void * worker(void *arg);
    void run();

    int m_threadnum;
    int m_actor
    int m_max_request;

    std::queue<T*> m_work_queue;
    Locker m_mutex;
    Sem m_sem;
    Cond m_cond;
};

#endif