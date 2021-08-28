#ifndef WEBSERVER_THREADPOOL_HPP
#define WEBSERVER_THREADPOOL_HPP

#include <unistd.h>
#include <pthread.h>

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

}

#endif //WEBSERVER_THREADPOOL_HPP
