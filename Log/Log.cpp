#include "Log.h"

Log::Log() {}

Log::~Log() {}

void Log::init(std::string path, bool uselog, int log_buf_size, int split_line, int max_queue_size)
{
    if(max_queue_size > 0)
    {
        m_is_async = true;
        pthread_t tid;
        pthread_create(&tid, NULL, flush_log_threaad, NULL);
    }
}

void *Log::flush_log_threaad(void *args)
{
    Log::get_instance()->async_write_log();
}

void *Log::async_write_log()
{
    std::string single_log;
    while (m_log_que->pop(single_log))
    {
        m_mutex.lock();
        fputs(single_log.c_str(), m_fp);
        m_mutex.unlock();
    }
}