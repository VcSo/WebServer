#ifndef WEBSERVER_HTTPCONN_H
#define WEBSERVER_HTTPCONN_H

#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <map>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/uio.h>
#include <mysql/mysql.h>

#include "../Log/Log.h"
#include "../Timer/Timer.h"

class Http {
public:
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };

    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };

    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;

    Http();
    ~Http();

    void init(int sockfd, const struct sockaddr_in &addr, char *m_root, int conn_mode, int close_log,
                std::string username, std::string password, std::string database);

public:
    static int m_epollfd;
    static int m_user_count;

    int m_state;
    int improv;
    MYSQL *mysql;

private:
    void init();

    int m_sockfd;
    int m_conn_mode;
    int m_close_log;
    int bytes_to_end;
    int bytes_have_end;
    int m_content_length;
    int m_start_line;
    int m_checked_idx;
    int m_read_idx;
    int m_write_idx;
    int cgi;
    int timer_flag;

    char *doc_root;
    char *m_url;
    char *m_version;
    char *m_host;
    char m_read_buf[READ_BUFFER_SIZE];
    char m_write_buf[WRITE_BUFFER_SIZE];
    char m_real_file[FILENAME_LEN];

    std::string sql_username;
    std::string sql_password;
    std::string sql_database;

    bool m_linger;

    METHOD m_method;
    CHECK_STATE m_check_state;
    struct sockaddr_in m_addr;
    std::map<std::string, std::string> m_users;
};


#endif //WEBSERVER_HTTPCONN_H
