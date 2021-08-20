#ifndef WEBSERVER_CONNSQL_H
#define WEBSERVER_CONNSQL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include "../Log/Log.h"

class connSql {
public:
    static connSql *get_instance();
    void init(std::string host, std::string username, std::string password, std::string database, int sqlnum);

private:
    connSql();
    ~connSql();
    std::queue<MYSQL*> sql_que;

    int m_free_conn;
    int m_cur_conn;
    int m_sqlnum;

};

#endif //WEBSERVER_CONNSQL_H
