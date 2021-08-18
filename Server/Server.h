#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <time.h>
#include <mysql/mysql.h>

#include "../ThreadPool/ThreadPool.hpp"
#include "../Timer/Timer.h"
#include "./Epoll.h"
#include "../Http/Httpconn.h"
#include "../Sql/Sqlconn.h"

class WebServer {
public:
    WebServer(int port, int et, int timeout, bool optlinger,
              int sql_port, std::string sql_username, std::string sql_user_password, std::string sql_database,
              int conn_num, int thread_num, bool log_flag, int log_level, int log_queue_size);

private:
    int m_port;
    int m_et;
    int m_sql_port;
    int m_sql_conn_num;

    char *srcdir;

    std::string m_sql_username;
    std::string m_sql_password;
    std::string m_sql_database;

    time_t m_timeout;
    bool m_optlinger;

    ThreadPool<WebServer> *m_threadpool;
    Timer *m_timer;
    Epoll *m_epoll;

};


#endif //WEBSERVER_SERVER_H
