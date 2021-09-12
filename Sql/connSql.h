#ifndef CONNSQL_H
#define CONNSQL_H

#include <mysql/mysql.h>
#include <list>
#include <string>

#include "../Log/Log.h"

class ConnSql
{
public:
    static ConnSql * getinstance();

    void init(std::string localhost, std::string sql_username, std::string sql_password, std::string sql_database,
                    int sql_port, int max_conn, bool close_log);
    MYSQL *GetConnection();				 //获取数据库连接
    bool ReleaseConnection(MYSQL *conn); //释放连接
    int GetFreeConn();					 //获取连接
    void DestroyPool();

private:
    ConnSql();
    ~ConnSql();

    std::string m_sql_username;
    std::string m_sql_password;
    std::string m_sql_database;
    std::string m_localhost;

    int m_sql_threadnum;
    int m_max_conn;
    int m_sql_port;
    int m_cur_conn;
    int m_Free_conn;

    bool m_close_log;

    std::list<MYSQL *> connList;
    Locker m_mutex;
    Sem reverse;
};

class connectionRAII{
public:
    connectionRAII(MYSQL **con, ConnSql *connPool);
    ~connectionRAII();

private:
    MYSQL *conRAII;
    ConnSql *poolRAII;
};

#endif