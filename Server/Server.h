#ifndef SERVER_H
#define SERVER_H

#include <string>

#include "../Log/Log.h"

class Server {
public:
    Server();

    ~Server();

    Server(int port, std::string localhost, std::string sql_username, std::string sql_password, std::string sql_database,
                   int close_log, int lingermode, int et, int sql_threadnum, int threadnum, int actor_mode, int async);

    void set_log(std::string path);


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
};

#endif