#ifndef WEBSERVER_BLOCK_QUEUE_HPP
#define WEBSERVER_BLOCK_QUEUE_HPP

#include <queue>
#include <string>

#include "../Lock/Locker.h"

template <typename T>
class block_queue
{
public:
    block_queue();
    block_queue(int max_size);
    ~block_queue();

    bool push(T content);
    bool pop(T &content);

private:
    int m_size;
    T *m_array;
    int m_max_size;
    int m_front;
    int m_back;

    Cond m_cond;
    Locker m_mutex;
};

template <typename T>
block_queue<T>::block_queue(int max_size)
{
    m_max_size = max_size;
    m_array = new T[max_size];
    m_size = 0;
    m_front = -1;
    m_back = -1;

}

template <typename T>
block_queue<T>::~block_queue()
{
    m_mutex.lock();
    if(m_array != nullptr)
    {
        delete[] m_array;
    }
    m_mutex.unlock();
}

template <typename T>
bool block_queue<T>::pop(T &content)
{
    m_mutex.lock();
    while(m_size <= 0)
    {
        if(!m_cond.wait(m_mutex.get()))
        {
            m_mutex.unlock();
            return false;
        }
    }

    m_front = (m_front + 1) % m_max_size;
    content = m_array[m_front];
    m_size--;
    m_mutex.unlock();

    return true;
}



#endif //WEBSERVER_BLOCK_QUEUE_HPP
