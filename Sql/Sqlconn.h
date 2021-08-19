#ifndef nWEBSERVER_SQLCONN_H
#defie WEBSERVER_SQLCONN_H

#include <queue>
#include <mysql/mysql.h>
#include <unistd.h>

class SqlConnPool {
public:
    SqlConnPool();
    ~SqlConnPool();
    static SqlConnPool *Instance();

    void init(const char *host, int port, std::string username, std::string password, std::string database, int sqlconn_num);

private:
    std::queue<MYSQL *> m_connQue;

    int MAX_CONN;
    int useCount;
    int freeCount;
};


#endif //WEBSERVER_SQLCONN_H
