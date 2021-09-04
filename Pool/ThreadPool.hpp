#ifndef WEBSERVER_THREADPOOL_HPP
#define WEBSERVER_THREADPOOL_HPP

#include <unistd.h>
#include <pthread.h>
#include <list>

#include "../Log/Log.h"
#include "../Lock/Locker.h"
#include "../Sql/connSql.h"

template <typename T>
class ThreadPool
{
public:
    ThreadPool(int actor, connSql *m_sql, int threadnum = 8, int max_request = 10000);
    ~ThreadPool();


private:
    static void* worker(void *arg);
    void run();

    pthread_t *m_threads;

    int m_actor;
    int m_thread_num;

    std::list<T*> m_queue;

    Sem m_queue_stat;
    Locker m_mutex;
};

template <typename T>
ThreadPool<T>::ThreadPool(int actor, connSql *m_sql, int threadnum, int max_request) : m_actor(actor), m_thread_num(threadnum)
{
    if(threadnum <= 0 || max_request <= 0)
    {
        throw std::exception();
    }

    m_threads = new pthread_t[m_thread_num];
    if(!m_threads)
    {
        throw std::exception();
    }

    for(int i = 0; i < m_thread_num; i++)
    {
        if(pthread_create(m_threads + i, nullptr, worker, this) != 0)
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
ThreadPool<T>::~ThreadPool()
{
    delete[] m_threads;
}

template <typename T>
void* ThreadPool<T>::worker(void *arg)
{
    //ThreadPool *pool = (ThreadPool *)arg;
    ThreadPool *pool = static_cast<ThreadPool *>(arg);
    pool->run();
    return pool;
}

template <typename T>
void ThreadPool<T>::run()
{
    while(true)
    {

        m_queue_stat.wait();
        m_mutex.lock();

        if(m_queue.empty())
        {
            m_mutex.unlock();
            continue;
        }

        T *request = m_queue.front();
        m_queue.pop_front();
        m_mutex.unlock();

        if(!request)
        {
            continue;
        }

        if(m_actor == 1)
        {
            if(request->m_state == 0)
            {
                request->improv = 1;
            }
        }
    }
}

#endif //WEBSERVER_THREADPOOL_HPP
