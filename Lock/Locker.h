#ifndef LOCKER_H
#define LOCKER_H

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <exception>
class Sem
{
public:
    Sem();
    ~Sem();

    Sem(int num);

    bool wait();
    bool post();

private:
    sem_t m_sem;
};

class Cond
{
public:
    Cond();
    ~Cond();

    bool wait(pthread_mutex_t *m_mutex);
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t);
    bool signal();
    bool broadcast();

private:

    pthread_cond_t m_cond;
};

class Locker
{
public:
    Locker();
    ~Locker();

    bool lock();
    bool unlock();
    pthread_mutex_t *get()
    {
        return &m_mutex;
    }

private:

    pthread_mutex_t m_mutex;
};

#endif