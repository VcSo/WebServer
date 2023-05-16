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
#include <sys/uio.h>
#include <sys/mman.h>
#include <netinet/tcp.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../Log/Log.h"
#include "../Lock/Locker.h"
#include "../Sql/connSql.h"

class Http
{
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 20480;
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
        SUCCESS_JSON,
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
    void init_mysqlresult(ConnSql *m_sql);
    void process();
    void close_conn(bool real_close = true);

    bool read_once();
    bool write();

    sockaddr_in * get_address();

private:
    void init();
    void unmap();

    bool process_write(HTTP_CODE ret);
    bool add_status_line(int status, const char *title);
    bool add_response(const char *format, ...);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();
    bool add_content(const char *content);

    LINE_STATUS parse_line();
    HTTP_CODE process_read();
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();

    char *get_line() { return m_read_buf + m_start_line; }; //\r \n \0

private:
    int m_sockfd;
    int m_conn_mode;
    int cgi;
    int m_write;
    int m_write_idx;
    int m_read;
    //缓冲区中m_read_buf中数据的最后一个字节的下一个位置
    int m_read_idx;
    //m_read_buf读取的位置m_checked_idx
    int m_checked_idx;
    //m_read_buf中已经解析的字符个数
    int m_start_line;
    //需要发送的数据大小
    int bytes_to_send;
    //已经发送的数据大小
    int bytes_have_send;
    int m_content_length;
    int m_iv_count;

    bool m_close_log;
    bool m_linger;
    bool m_write_json;

    //存储接收到的数据
    char m_read_buf[READ_BUFFER_SIZE];
    //存储发出的响应报文数据
    char m_write_buf[WRITE_BUFFER_SIZE];
    char m_real_file[FILENAME_LEN];

    char *m_root;
    char *m_url;
    char *m_version;
    char *m_host;
    char *m_file_address;
    char *m_string; //存储请求头数据

    std::string m_sql_username;
    std::string m_sql_password;
    std::string m_sql_database;
    std::string m_filename;
    std::string m_json;

    struct sockaddr_in m_addr;
    struct iovec m_iv[2];
    struct stat m_file_stat;

    CHECK_STATE m_check_state;
    METHOD m_method;
    Locker m_mutex;
    Sem m_sem;
};

#endif