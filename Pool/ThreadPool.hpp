#ifndef ThreadPool_H
#define ThreadPool_H

#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <queue>

#include "../Sql/connSql.h"
#include "../Lock/Locker.h"

template <typename T>
class ThreadPool
{
public:
    ThreadPool(int actor, ConnSql *sql, int threadnum, int max_request = 10000);
    ~ThreadPool();

    bool append(T *request, int state);
    bool append_p(T *request);

private:
    static void * worker(void *arg);
    void run();

    int m_threadnum;
    int m_actor;
    int m_max_request;

    pthread_t *m_threads;

    std::queue<T*> m_work_queue;
    Locker m_mutex;
    Sem m_sem;
    Cond m_cond;
    ConnSql *m_sql;
};

#endif

#include <iostream>

template <typename T>
ThreadPool<T>::ThreadPool(int actor, ConnSql *sql, int threadnum, int max_request) : m_actor(actor), m_sql(sql), m_threadnum(threadnum), m_max_request(max_request), m_threads(nullptr)
{
    if(m_threadnum <= 0 || m_max_request <= 0)
    {
        throw std::exception();
    }

    m_threads = new pthread_t[m_threadnum];
    if(!m_threads)
    {
        throw std::exception();
    }

    for(int i = 0; i < m_threadnum; ++i)
    {
        if(pthread_create(&m_threads[i], nullptr, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }

        if(pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
void * ThreadPool<T>::worker(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool *>(arg);
    pool->run();
    return pool;
}

template <typename T>
void ThreadPool<T>::run()
{
    while(true)
    {
        m_sem.wait();
        m_mutex.lock();
        if(m_work_queue.empty())
        {
            m_mutex.unlock();
            continue;
        }

        T *request = m_work_queue.front();
        m_work_queue.pop();
        if(!request)
        {
            continue;
        }

        if(m_actor == 1)
        {
            if(request->m_state == 0)
            {
//                if(request->read_once())
//                {
//
//                }
            }
        }
    }
}

template <typename T>
bool ThreadPool<T>::append(T *request, int state)
{
    m_mutex.lock();
    if(m_work_queue.size() >= m_max_request)
    {
        m_mutex.unlock();
        return false;
    }
    request->m_state = state;
    m_work_queue.push(request);
    m_mutex.unlock();
    m_sem.post();
    return true;
}

template <typename T>
bool ThreadPool<T>::append_p(T *request)
{
    m_mutex.lock();
    if(m_work_queue.size() >= m_max_request)
    {
        m_mutex.unlock();
        return false;
    }

    m_work_queue.push(request);
    m_mutex.unlock();
    m_sem.post();
    return true;
}