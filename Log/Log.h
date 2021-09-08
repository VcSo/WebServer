#ifndef WEBSERVER_LOG_H
#define WEBSERVER_LOG_H

#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <sys/time.h>
#include <stdarg.h>
#include <time.h>
#include <chrono>

#include "./Block_queue.hpp"

namespace vc
{

}
class Log {
public:
    static Log *get_instance();
    static void *flush_log_threaad(void *args);

    bool init(std::string path, bool uselog, int log_buf_size, int split_line, int max_queue_size);

    void flush();
    void write_log(int level, const char *format, ...);

private:
    Log();
    virtual ~Log();
    void *async_write_log();

    std::string m_logfile_name;
    std::string m_fileline;

    std::ofstream writelog;

    int m_today; //因为按天分类,记录当前时间是那一天
    int m_split_lines;  //日志最大行数
    int m_log_buf_size; //日志缓冲区大小

    bool m_close_log;
    bool m_is_async;

    long long m_count;  //日志行数记录
    char *m_buf;

    block_queue<std::string> *m_log_que;
    Locker m_mutex;
};

#define LOG_DEBUG(format, ...) if(1 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(1 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(1 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(1 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif //WEBSERVER_LOG_H
