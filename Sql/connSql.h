#ifndef CONNSQL_H
#define CONNSQL_H

#include <mysql/mysql.h>
#include <list>

#include "../Log/Log.h"

class ConnSql
{
public:
    static ConnSql * getinstance();

    void init(std::string localhost, std::string sql_username, std::string sql_password, std::string sql_database,
                    int sql_port, int max_conn, bool close_log);
    MYSQL *GetConnection();				 //获取数据库连接
    int GetFreeConn();					 //获取连接
    void DestroyPool();
    void ReleaseConnection(MYSQL *con); //释放连接

private:
    ConnSql(); //单例模式，只允许创建一个对象，不允许外部创建对象
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