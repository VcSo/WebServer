#include "Server.h"

WebServer::WebServer(int port, int et, int timeout, bool optlinger,
          int sql_port, std::string sql_username, std::string sql_user_password, std::string sql_database,
          int sqlconn_num, int thread_num, bool log_flag, int log_level, int log_queue_size)
          : m_port(port), m_et(et), m_timeout(timeout), m_optlinger(optlinger),
            m_sql_port(sql_port), m_sql_username(sql_username), m_sql_password(sql_user_password), m_sql_database(sql_database),
            m_sql_conn_num(sqlconn_num), m_threadpool(new ThreadPool<WebServer>(thread_num)), m_timer(new Timer()), m_epoll(new Epoll())
{
    srcdir = getcwd(nullptr, 256);
    static_assert(srcdir, "srcdir == null");
    strncat(srcdir,  "/resources/", 16);
    std::cout << srcdir << std::endl;

    Httpconn::srcDir = srcdir;
    Httpconn::userCount = 0;

    SqlConnPool::Instance()->init("localhost", m_sql_port, m_sql_username, m_sql_password, m_sql_database, m_sql_conn_num);

}
