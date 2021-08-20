#ifndef WEBSERVER_LOG_H
#define WEBSERVER_LOG_H

#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>

class Log {
public:
    static Log *get_instance();
    void init(std::string path, bool uselog, int log_buf_size, int split_line, int max_queue_size);

private:
    Log();
    ~Log();

    std::string m_logfile_name;
    std::string m_fileline;

    std::ofstream writelog;

    bool m_is_async;
};


#endif //WEBSERVER_LOG_H
