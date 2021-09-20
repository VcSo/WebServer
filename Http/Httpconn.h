#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <map>
#include <string>
#include <cassert>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/types.h>

#include "../Log/Log.h"
#include "../Lock/Locker.h"
#include "../Sql/connSql.h"

class Http
{
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

    static int m_user_count;
    static int m_epollfd;
    int m_state;
    int improv;
    int timer_flag;

    MYSQL *mysql;

public:
    Http();
    ~Http();

    void init(int connfd, struct sockaddr_in client_addr, char *root, int conn_mode, bool close_log,
                            std::string sql_username, std::string sql_password, std::string sql_database);
    void init();
    void init_mysqlresult(ConnSql *m_sql);

    bool read_once();
    bool write();

    sockaddr_in * get_address();

private:
    int m_sockfd;
    int m_conn_mode;
    int cgi;
    int m_write;
    int m_write_idx;
    int m_read;
    int m_read_idx;
    int m_checked_idx;
    int m_start_line;
    int bytes_to_send;
    int bytes_have_send;
    int m_content_length;

    bool m_close_log;
    bool m_linger;

    char m_read_buf[READ_BUFFER_SIZE];
    char m_write_buf[WRITE_BUFFER_SIZE];
    char m_real_file[FILENAME_LEN];
    char *m_root;
    char *m_url;
    char *m_version;
    char *m_host;

    std::string m_sql_username;
    std::string m_sql_password;
    std::string m_sql_database;

    struct sockaddr_in m_addr;

    CHECK_STATE m_check_state;
    METHOD m_method;
    Locker m_mutex;
    Sem m_sem;
};

#endif