#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <iostream>

#include "../Log/Log.h"
#include "../Sql/connSql.h"
#include "../Http/Httpconn.h"
#include "../Pool/ThreadPool.hpp"
#include "../Timer/Timer.h"

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位

class Server {
public:
    Server();
    ~Server();

    Server(int port, std::string localhost, std::string sql_username, std::string sql_password, std::string sql_database,
                   bool close_log, int lingermode, int et, int sql_threadnum, int threadnum, int actor_mode, int async);

    void Start();
    void threadpool();
    void trig_mode();
    void setsql();
    void event_listen();
    void set_log(std::string path);
    void timer(int connfd, struct sockaddr_in client_addr);
    void adjust_timer(util_timer *timer);
    void deal_timer(util_timer *timer, int sockfd);
    void dealwithread(int sockfd);

    bool dealclientdata();
    bool dealwithsignal(bool& timeout, bool& stop_server);

private:
    std::string m_localhost;
    std::string m_sql_username;
    std::string m_sql_password;
    std::string m_sql_database;

    char *m_root;

    bool m_close_log;

    int m_port;
    int m_lingermode;
    int m_et;
    int m_sqlthreadnum;
    int m_threadnum;
    int m_actor_mode;
    int m_async;
    int m_listen_mode;
    int m_conn_mode;

    int m_pipefd[2];
    int m_listenfd;
    int m_epollfd;

    epoll_event events[MAX_EVENT_NUMBER];

    ConnSql *m_sql;
    Http *Users;
    ThreadPool<Http> *m_pool;
    Utils utils;
    client_data *users_timer;
};

#endif