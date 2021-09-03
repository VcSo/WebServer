#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/epoll.h>

#include "../Sql/connSql.h"
#include "../Pool/ThreadPool.hpp"
#include "../Http/Httpconn.h"

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;

class Server {
public:
    Server(int port, std::string m_host, std::string username, std::string password, std::string database,
                   int uselog, int linger, int mode, int sqlnum, int threadnum, int actor, int logmode);
    ~Server();

    void setsql();
    void setlog(std::string path);
    void Start();
    void threadpool();
    void trig_mode();
    void event_listen();
    void timer(int connfd, struct sockaddr_in client_addr);

    bool deal_client_data();

private:
    int m_port;
    int m_linger;
    int et;
    int m_sql_num;
    int m_thread_num;
    int m_actor;
    int m_log_mode;
    int m_listen_mode;
    int m_conn_mode;
    int m_listenfd;
    int m_connfd;
    int m_epollfd;
    int m_pipefd[2];
    int m_close_log;

    std::string m_host;
    std::string m_username;
    std::string m_password;
    std::string m_database;
    std::string m_log_path;

    //epoll_event相关
    epoll_event events[MAX_EVENT_NUMBER];

    connSql *m_consql;
    ThreadPool<Http> *m_pool;
    Utils utils;
    Http *users;
    client_data *users_timer;
};

#endif //WEBSERVER_SERVER_H
