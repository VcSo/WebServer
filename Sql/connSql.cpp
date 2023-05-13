#include "connSql.h"

ConnSql * ConnSql::getinstance()
{
    static ConnSql connsql;
    return &connsql;
}

ConnSql::ConnSql() : m_Free_conn(0), m_cur_conn(0)
{
}

ConnSql::~ConnSql()
{
    DestroyPool();
}

void ConnSql::DestroyPool()
{
    m_mutex.lock();
    if(connList.size() > 0)
    {
        for(MYSQL *con : connList)
        {
            mysql_close(con);
        }

        m_cur_conn = 0;
        m_Free_conn = 0;
        connList.clear();
    }

    m_mutex.unlock();
}

void ConnSql::init(std::string localhost, std::string sql_username, std::string sql_password, std::string sql_database,
                        int sql_port, int max_conn, bool close_log)
{
    m_localhost = localhost;
    m_sql_username = sql_username;
    m_sql_password = sql_password;
    m_sql_database = sql_database;
    m_sql_threadnum = max_conn;
    m_sql_port = sql_port;
    m_max_conn = max_conn;
    m_close_log = close_log;

    for(int i = 0; i < max_conn; ++i)
    {
        MYSQL *con = nullptr;
        con = mysql_init(con);

        if(con == nullptr)
        {
            LOG_ERROR("MYSQL Error");
            exit(1);
        }

        con = mysql_real_connect(con, m_localhost.c_str(), m_sql_username.c_str(), m_sql_password.c_str(), m_sql_database.c_str(), m_sql_port, nullptr, 0);
        if(con == nullptr)
        {
            LOG_ERROR("mysql error");
            exit(1);
        }

        connList.push_back(con);
        ++m_Free_conn;
    }

    reverse = Sem(m_Free_conn);
    m_max_conn = m_Free_conn;
}

connectionRAII::connectionRAII(MYSQL **SQL, ConnSql *connPool)
{
    *SQL = connPool->GetConnection();

    conRAII = *SQL;
    poolRAII = connPool;
}

connectionRAII::~connectionRAII()
{
    poolRAII->ReleaseConnection(conRAII);
}

MYSQL * ConnSql::GetConnection()
{
    MYSQL *con = NULL;

    if (0 == connList.size())
        return NULL;

    reverse.wait();

    m_mutex.lock();

    con = connList.front();
    connList.pop_front();

    --m_Free_conn;
    ++m_cur_conn;

    m_mutex.unlock();
    return con;
}

void ConnSql::ReleaseConnection(MYSQL *con)
{
    if (NULL == con)
    {
        return;
//        LOG_ERROR("release sqlconn error");
    }

    m_mutex.lock();

    connList.push_back(con);
    ++m_Free_conn;
    --m_cur_conn;

    m_mutex.unlock();

    reverse.post();
}