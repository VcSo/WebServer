#include "Log.h"

#include <sys/stat.h>
#include <sys/types.h>

static int file_num = 0;
char* setlogname(const std::string& log_path, char* logfile)
{
    auto fileExists = [](char* logfile) -> bool {
        std::ifstream ifile(logfile);
        return (bool)ifile;
    };
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char re_time[64] = {0};
    std::strftime(re_time, sizeof(re_time), "%Y_%m_%d", std::localtime(&now));
    snprintf(logfile, 255, "%s%s_%s_%d%s", log_path.c_str(), re_time, "Server", ++file_num, ".log");
    while(true)
    {
        if(fileExists(logfile))
        {
            snprintf(logfile, 255, "%s%s_%s_%d%s", log_path.c_str(), re_time, "Server", ++file_num, ".log");
            continue;
        }

        break;
    }
    return logfile;
}

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

    delete m_log_que;
    delete []m_buf;
}

Log *Log::get_instance()
{
    static Log log;
    return &log;
}

bool Log::init(std::string path, bool uselog, int log_buf_size, int split_line, int max_queue_size)
{
    std::cout << "log_init" << std::endl;
    m_split_lines = split_line;
    m_close_log = uselog;
    log_path = path;
    if(max_queue_size > 0)
    {
        m_is_async = true;
        m_log_que = new block_queue<std::string>(max_queue_size);
        pthread_t tid;
        pthread_create(&tid, NULL, flush_log_threaad, NULL);
    }

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    std::string logname;
    char logfile[256];
    m_today = my_tm.tm_mday;

    m_log_buf_size = log_buf_size;
    m_buf = new char[log_buf_size];
    memset(m_buf, '\0', m_log_buf_size);

    //https://blog.csdn.net/qq_38989148/article/details/107418683
    if(access(log_path.c_str(), F_OK) == -1 ){// 文件夹不存在, https://www.cnblogs.com/whwywzhj/p/7801409.html
        mkdir(log_path.c_str(), S_IRWXO|S_IRWXG|S_IRWXU);// https://www.jianshu.com/p/06a0da1f6389
    }
    setlogname(path, logfile);

//    snprintf(logfile, 255, "%s%d_%02d_%02d_%s", log_path.c_str(), my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, "Server.log");

    writelog.open(logfile, std::ios::binary);
    if(!writelog.is_open())
    {
        std::cout << "create log error" << std::endl;
        return false;
    }
    writelog << "create log" << "\n";

    return true;
}

void * Log::flush_log_threaad(void *args)
{
    Log::get_instance()->async_write_log();
}

void* Log::async_write_log()
{
    std::string single_log;
    while (m_log_que->pop(single_log))
    {
        m_mutex.lock();
        writelog << single_log << "\n";
        m_mutex.unlock();
    }
}



void Log::write_log(int level, const char *format, ...)
{
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char re_time[64] = {0};
    std::strftime(re_time, sizeof(re_time), "%Y-%m-%d_%X", std::localtime(&now));

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    char s_level[16] = {0};

    switch(level)
    {
        case 0:
            strcpy(s_level, "[DEBUG]: ");
            break;
        case 1:
            strcpy(s_level, "[INFO]: ");
            break;
        case 2:
            strcpy(s_level, "[WARNING]: ");
            break;
        case 3:
            strcpy(s_level, "[ERROR]: ");
            break;
        default:
            strcpy(s_level, "[INFO]: ");
            break;
    }

    int n = snprintf(m_buf, 64,"%s %s", re_time, s_level);
    m_mutex.lock();
    ++m_count;
    if (m_today != my_tm.tm_mday || m_count % m_split_lines == 0) //everyday log
    {
        std::cout << m_count << std::endl;

        writelog.close();
        if(m_today != my_tm.tm_mday)
        {
            time_t td = time(NULL);
            struct tm *tm_td = localtime(&td);
            struct tm my_tm = *tm_td;
            m_today = my_tm.tm_mday;
            file_num = 0;
            std::cout << m_today << std::endl;
            char new_log[256] = {0};
            setlogname(log_path, new_log);
            writelog.open(new_log, std::ios::binary);
            m_count = 0;
            if(!writelog.is_open())
            {
                //LOG_ERROR("Create new logfile error");
                writelog.open(new_log, std::ios::binary);
            }
        }
        else
        {

            char new_log[256] = {0};
            setlogname(log_path, new_log);
            writelog.open(new_log, std::ios::binary);
            m_count = 0;
            if(!writelog.is_open())
            {
                //LOG_ERROR("Create new logfile error");
                writelog.open(new_log, std::ios::binary);
            }
        }
    }

    m_mutex.unlock();

    va_list valst;
    va_start(valst, format);
    m_mutex.lock();

    int m = vsnprintf(m_buf + n, m_log_buf_size - 1, format, valst);
    m_buf[n + m] = '\n';
    m_buf[n + m + 1] = '\0';
    std::string log_str = m_buf;

    m_mutex.unlock();

    if(m_is_async && !m_log_que->full())
    {
        m_log_que->push(log_str);
    }
    else
    {
        m_mutex.lock();
        writelog << log_str;
        m_mutex.unlock();
    }

    va_end(valst);
}

void Log::flush()
{
    m_mutex.lock();
    writelog.flush();
    m_mutex.unlock();
}
