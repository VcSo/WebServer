#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <time.h>
#include <mysql/mysql.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <memory>
#include <fcntl.h>

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

    void InitEventMode(int trigMode);
    void setNonblock(int fd);
    void Start();

    bool InitSocket();

private:
    char *srcdir;

    int m_port;
    int m_et;
    int m_sql_port;
    int m_sql_conn_num;
    int listenfd;

    bool m_optlinger;
    bool isClose;

    uint32_t listenEvent;
    uint32_t connEvent;
    time_t m_timeout;

    std::string m_sql_username;
    std::string m_sql_password;
    std::string m_sql_database;

private:
    ThreadPool<WebServer> *m_threadpool;
    Timer *m_timer;
    Epoll *m_epoll;
    std::unique_ptr<Epoll> epoller;
    std::unique_ptr<Timer> timer;
};


#endif //WEBSERVER_SERVER_H
