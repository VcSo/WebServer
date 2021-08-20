#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include <string>
#include "../Sql/connSql.h"

class Server {
public:
    Server(int port, std::string m_host, std::string username, std::string password, std::string database,
                   bool uselog, int linger, int mode, int sqlnum, int threadnum, int actor, int logmode);
    ~Server();

    void setsql();
    void setlog(std::string path);
    //void Start();


private:
    int m_port;
    int m_linger;
    int et;
    int m_sql_num;
    int m_thead_num;
    int m_actor;
    int m_log_mode;

    std::string m_host;
    std::string m_username;
    std::string m_password;
    std::string m_database;
    std::string m_log_path;

    connSql *m_consql;
    bool m_uselog;
};

#endif //WEBSERVER_SERVER_H
