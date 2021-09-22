#include "Httpconn.h"

#include <iostream>

//定义http响应的一些状态信息
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

int Http::m_user_count = 0;
int Http::m_epollfd = -1;
std::map<std::string, std::string> users;

Http::Http() : m_close_log(true)
{

}

Http::~Http()
{

}

//对文件描述符设置非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option); //设置文件状态标记
    return old_option;
}

void addfd(int epollfd, int sockfd, bool one_shot, int conn_mode)
{
    epoll_event event;
    event.data.fd = sockfd;

    if (1 == conn_mode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if(one_shot)
        event.events |= EPOLLONESHOT; //EPOLLONESHOT：只监听一次事件
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
    setnonblocking(sockfd);
}

void Http::init(int connfd, struct sockaddr_in client_addr, char *root, int conn_mode, bool close_log,
                            std::string sql_username, std::string sql_password, std::string sql_database)
{
    m_sockfd = connfd;
    m_addr = client_addr;
    m_root = root;
    m_conn_mode = conn_mode;
    m_close_log = close_log;
    m_sql_username = sql_username;
    m_sql_password = sql_password;
    m_sql_database = sql_database;

    addfd(m_epollfd, m_sockfd, true, m_conn_mode);
    ++m_user_count;

    init();
}

void Http::init()
{
    mysql = NULL;
    bytes_to_send = 0;
    bytes_have_send = 0;
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_content_length = 0;
    m_host = 0;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    cgi = 0;
    m_state = 0;
    timer_flag = 0;
    improv = 0;

    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}

void Http::init_mysqlresult(ConnSql *m_sql)
{
    MYSQL *mysql = nullptr;
    connectionRAII mysqlcon(&mysql, m_sql);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);

    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        std::string temp1(row[0]);
        std::string temp2(row[1]);
        users[temp1] = temp2;
    }
}

bool Http::read_once()
{
    if (m_read_idx >= READ_BUFFER_SIZE)
    {
        return false;
    }
    int bytes_read = 0;

    //LT读取数据
    if (0 == m_conn_mode)
    {
        bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        m_read_idx += bytes_read;

        if (bytes_read <= 0)
        {
            return false;
        }

        return true;
    }
    //ET读数据
    else
    {
        while (true)
        {
            bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
            if (bytes_read == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                return false;
            }
            else if (bytes_read == 0)
            {
                return false;
            }
            m_read_idx += bytes_read;
        }
        return true;
    }
}

sockaddr_in * Http::get_address()
{
    return &m_addr;
}

bool Http::write()
{
    int temp = 0;

//    if (bytes_to_send == 0)
//    {
//        modfd(m_epollfd, m_sockfd, EPOLLIN, m_conn_mode);
//        init();
//        return true;
//    }
//
//    while (1)
//    {
//        temp = writev(m_sockfd, m_iv, m_iv_count);
//
//        if (temp < 0)
//        {
//            if (errno == EAGAIN)
//            {
//                modfd(m_epollfd, m_sockfd, EPOLLOUT, m_conn_mode);
//                return true;
//            }
//            unmap();
//            return false;
//        }
//
//        bytes_have_send += temp;
//        bytes_to_send -= temp;
//        if (bytes_have_send >= m_iv[0].iov_len)
//        {
//            m_iv[0].iov_len = 0;
//            m_iv[1].iov_base = m_file_address + (bytes_have_send - m_write_idx);
//            m_iv[1].iov_len = bytes_to_send;
//        }
//        else
//        {
//            m_iv[0].iov_base = m_write_buf + bytes_have_send;
//            m_iv[0].iov_len = m_iv[0].iov_len - bytes_have_send;
//        }
//
//        if (bytes_to_send <= 0)
//        {
//            unmap();
//            modfd(m_epollfd, m_sockfd, EPOLLIN, m_conn_mode);
//
//            if (m_linger)
//            {
//                init();
//                return true;
//            }
//            else
//            {
//                return false;
//            }
//        }
//    }
}

void Http::process()
{
    HTTP_CODE read_ret = process_read();
}

void Http::process_read()
{
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text = 0;

    while ((m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) || ((line_status = parse_line()) == LINE_OK))
    {
        text = get_line();
        m_start_line = m_checked_idx;
        LOG_INFO("%s", text);
        switch(m_check_state)
        {
            case CHECK_STATE_REQUESTLINE:
            {
                ret = parse_request_line(text);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                break;
            }
            case CHECK_STATE_HEADER:
            {
                ret = parse_headers(text);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                else if (ret == GET_REQUEST)
                {
                    return do_request();
                }
                break;
            }
            case CHECK_STATE_CONTENT:
            {
                ret = parse_content(text);
                if (ret == GET_REQUEST)
                    return do_request();
                line_status = LINE_OPEN;
                break;
            }
            default:
                return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}

LINE_STATUS Http::parse_line()
{
    char temp;
    for (; m_checked_idx < m_read_idx; ++m_checked_idx)
    {
        temp = m_read_buf[m_checked_idx];

        if (temp == '\r')
        {
            if ((m_checked_idx + 1) == m_read_idx)
                return LINE_OPEN;
            else if (m_read_buf[m_checked_idx + 1] == '\n')
            {
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (temp == '\n')
        {
            if (m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r')
            {
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }

    return LINE_OPEN;
}

//解析http请求行，获得请求方法，目标url及http版本号
HTTP_CODE Http::parse_request_line(char *text)
{
    m_url = strpbrk(text, " \t");
    if (!m_url)
    {
        return BAD_REQUEST;
    }
    *m_url++ = '\0';
    char *method = text;
    if (strcasecmp(method, "GET") == 0)
        m_method = GET;
    else if (strcasecmp(method, "POST") == 0)
    {
        m_method = POST;
        cgi = 1;
    }
    else
        return BAD_REQUEST;
    m_url += strspn(m_url, " \t");
    m_version = strpbrk(m_url, " \t");
    if (!m_version)
        return BAD_REQUEST;
    *m_version++ = '\0';
    m_version += strspn(m_version, " \t");
    if (strcasecmp(m_version, "HTTP/1.1") != 0)
        return BAD_REQUEST;
    if (strncasecmp(m_url, "http://", 7) == 0)
    {
        m_url += 7;
        m_url = strchr(m_url, '/');
    }

    if (strncasecmp(m_url, "https://", 8) == 0)
    {
        m_url += 8;
        m_url = strchr(m_url, '/');
    }

    if (!m_url || m_url[0] != '/')
        return BAD_REQUEST;
    //当url为/时，显示判断界面
    if (strlen(m_url) == 1)
        strcat(m_url, "judge.html");
    m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}