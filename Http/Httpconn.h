#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <map>
#include <string>
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
    static int m_user_count;
    static int m_epollfd;
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

public:
    Http();
    ~Http();

    void init_mysqlresult(ConnSql *m_sql);
    bool read_once();

private:
    int m_state;
    int m_write;
    int m_read;
    int m_read_idx;
    int m_checked_idx;
    int m_start_line;

    bool m_close_log;

    Locker m_mutex;

};

#endif