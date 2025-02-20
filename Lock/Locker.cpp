#include "Locker.h"

Sem::Sem()
{
    if(sem_init(&m_sem, 0, 0) != 0)
    {
        throw std::exception();
    }

}

Sem::Sem(int num)
{
    if(sem_init(&m_sem, 0, num) != 0)
    {
        throw std::exception();
    }
}

Sem::~Sem()
{
    sem_destroy(&m_sem);
}

bool Sem::wait()
{
    return sem_wait(&m_sem) == 0;
}

bool Sem::post()
{
    return sem_post(&m_sem) == 0;
}

Cond::Cond()
{
    if (pthread_cond_init(&m_cond, NULL) != 0)
    {
        throw std::exception();
    }
}

Cond::~Cond()
{
    pthread_cond_destroy(&m_cond);
}

bool Cond::wait(pthread_mutex_t *m_mutex)
{
    int ret = 0;
    ret = pthread_cond_wait(&m_cond, m_mutex);
    return ret == 0;
}

bool Cond::timewait(pthread_mutex_t *m_mutex, struct timespec t)
{
    int ret = 0;
    ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
    return ret == 0;
}

bool Cond::signal()
{
    return pthread_cond_signal(&m_cond) == 0;
}

bool Cond::broadcast()
{
    return pthread_cond_broadcast(&m_cond) == 0;
}

Locker::Locker()
{
    if (pthread_mutex_init(&m_mutex, NULL) != 0)
    {
        throw std::exception();
    }
}

Locker::~Locker()
{
    pthread_mutex_destroy(&m_mutex);
}

bool Locker::lock()
{
    return pthread_mutex_lock(&m_mutex) == 0;
}

bool Locker::unlock()
{
    return pthread_mutex_unlock(&m_mutex) == 0;
}