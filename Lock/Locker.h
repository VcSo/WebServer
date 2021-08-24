#ifndef WEBSERVER_LOCKER_H
#define WEBSERVER_LOCKER_H

#include <pthread.h>
#include <mutex>
#include <semaphore.h>

class Sem
{
public:
    Sem();
    Sem(int num);
    ~Sem();

    bool wait();
    bool post();

private:
    sem_t m_sem;

};

class Locker {
public:
    Locker();
    ~Locker();

    pthread_mutex_t *get()
    {
        return &m_mutex;
    }
    bool lock();
    bool unlock();

private:
    pthread_mutex_t m_mutex;

};

class Cond
{
public:
    Cond();
    ~Cond();

    bool wait(pthread_mutex_t *m_mutex);
    bool timewait(pthread_mutex_t *mutex, struct timespec t);
    bool signal();
    bool broadcast();

private:
    pthread_cond_t m_cond;

};

#endif //WEBSERVER_LOCKER_H
