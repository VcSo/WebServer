#include "Log.h"

Log::Log()
{
    m_count = 0;
    m_is_async = false;
}

Log::~Log()
{

    if(writelog.is_open())
    {
        writelog.close();
    }
}

void Log::init(std::string path, bool uselog, int log_buf_size, int split_line, int max_queue_size)
{
    if(max_queue_size > 0)
    {
        m_is_async = true;
        pthread_t tid;
        //pthread_create(&tid, NULL, flush_log_threaad, NULL);
    }

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    std::string logname;
    char logfile[256];
    snprintf(logfile, 255, "%s%d_%02d_%02d_%s", path.c_str(), my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, "server.log");

    writelog.open(logfile, std::ios::app);
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
        writelog << single_log;
        m_mutex.unlock();
    }
}