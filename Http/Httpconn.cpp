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

//将事件重置为EPOLLONESHOT
void modfd(int epollfd, int fd, int ev, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
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
    //将被监听的描述符添加到epoll句柄或从epool句柄中删除或者对监听事件进行修改
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
    m_write_json = false;
    m_filename = "";
    m_json = "";

    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}

void Http::init_mysqlresult(ConnSql *m_sql)
{
    MYSQL *mysql = nullptr;
    connectionRAII mysqlcon(&mysql, m_sql);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,password FROM user"))
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
        //int recv(SOCKET s, char *buf, int len, int flags);
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

    if(bytes_to_send == 0)
    {
        modfd(m_epollfd, m_sockfd, EPOLLIN, m_conn_mode);
        init();
        return true;
    }

    while(true)
    {
        temp = writev(m_sockfd, m_iv, m_iv_count);

        if(temp < 0)
        {
            if(errno == EAGAIN)
            {
                modfd(m_epollfd, m_sockfd, EPOLLOUT, m_conn_mode);
                return true;
            }
            unmap();
            return false;
        }

        bytes_have_send += temp;
        bytes_to_send -= temp;
        if(m_write_json)
        {
            m_iv[0].iov_len = 0;
            //将文件中未发送的数据写入缓冲区
            m_iv[1].iov_base = (char*)m_json.c_str() + (bytes_have_send - m_write_idx);
            m_iv[1].iov_len = bytes_to_send;
        }
        else if(bytes_have_send >= m_write_idx)
        {
            m_iv[0].iov_len = 0;
            //将文件中未发送的数据写入缓冲区
            m_iv[1].iov_base = m_file_address + (bytes_have_send - m_write_idx);
            m_iv[1].iov_len = bytes_to_send;
        }
        else
        {
            m_iv[0].iov_base = m_write_buf + bytes_have_send;
//            m_iv[0].iov_len = m_iv[0].iov_len - bytes_have_send;
            m_iv[0].iov_len = m_iv[0].iov_len - temp;
        }

        if(bytes_to_send <= 0)
        {
            unmap();
            modfd(m_epollfd, m_sockfd, EPOLLIN, m_conn_mode);

            if (m_linger && !m_write_json)
            {
                init();
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}

void Http::process()
{
    HTTP_CODE read_ret = process_read();
    if (read_ret == NO_REQUEST)
    {
        modfd(m_epollfd, m_sockfd, EPOLLIN, m_conn_mode);
        return;
    }

    bool write_ret = process_write(read_ret);
    if (!write_ret)
    {
        close_conn();
    }

    modfd(m_epollfd, m_sockfd, EPOLLOUT, m_conn_mode);
}

//请求报文进行解析
//GET / HTTP/1.1\r\nHost: www.baidu.com.cn\r\nConnection: close\r\n\r\n
Http::HTTP_CODE Http::process_read()
{
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text = nullptr;

    while((m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) || (line_status = parse_line()) == LINE_OK)
    {
        text = get_line();
        std::cout << text << std::endl;
        m_start_line = m_checked_idx;
        switch (m_check_state)
        {
            case CHECK_STATE_REQUESTLINE: //解析请求行
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

bool Http::process_write(Http::HTTP_CODE ret)
{
    switch(ret)
    {
        case INTERNAL_ERROR:
        {
            add_status_line(500, error_500_title);
            add_headers(strlen(error_500_form));
            if (!add_content(error_500_form))
                return false;
            break;
        }
        case NO_RESOURCE:
        {
            add_status_line(404, error_404_title);
            add_headers(strlen(error_404_form));
            if (!add_content(error_404_form))
                return false;
            break;
        }
        case FORBIDDEN_REQUEST:
        {
            add_status_line(403, error_403_title);
            add_headers(strlen(error_403_form));
            if (!add_content(error_403_form))
                return false;
            break;
        }
        case FILE_REQUEST:
        {
            add_status_line(200, ok_200_title);
            if (m_file_stat.st_size != 0)
            {
                add_headers(m_file_stat.st_size);
                m_iv[0].iov_base = m_write_buf;
                m_iv[0].iov_len = m_write_idx;
                m_iv[1].iov_base = m_file_address;
                m_iv[1].iov_len = m_file_stat.st_size;
                m_iv_count = 2;
                bytes_to_send = m_write_idx + m_file_stat.st_size;
                return true;
            }
            else
            {
                const char *ok_string = "<html><body></body></html>";
                add_headers(strlen(ok_string));
                if (!add_content(ok_string))
                    return false;
            }
        }
        case SUCCESS_JSON:
        {
            add_status_line(200, ok_200_title);
            m_json = "{\"code\": 200,\"success\": true,\"msg\": \"上传成功\",\"fileName\":\"./SaveFile/" + m_filename + "\"}";

//            char buf[128];
//            memset(buf, 0, sizeof(buf));
//            int n = snprintf(buf, 128, "%s", str.c_str());
            int str_len = m_json.length();
            add_headers(str_len);
            m_iv[0].iov_base = m_write_buf;
            m_iv[0].iov_len = m_write_idx;
            m_iv[1].iov_base = (char*)m_json.c_str();
            m_iv[1].iov_len = str_len;
            m_iv_count = 2;
            bytes_to_send = m_write_idx + str_len;
            m_write_json = true;
            return true;
        }
        default:
            return false;
    }

    return false;
}

Http::HTTP_CODE Http::parse_content(char *text)
{
//    std::cout << text << std::endl;
    if(m_read_idx >= (m_content_length + m_checked_idx))
    {
        text[m_content_length] = '\0';
        //POST请求中最后为输入的用户名和密码
        m_string = text;
        return GET_REQUEST;
    }

    return NO_REQUEST;
}

Http::HTTP_CODE Http::parse_headers(char *text)
{
//    std::cout << text << std::endl;
    if (text[0] == '\0')
    {
        if (m_content_length != 0)
        {
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0)
    {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0)
        {
            m_linger = true;
        }
    }
    else if (strncasecmp(text, "Content-length:", 15) == 0)
    {
        text += 15;
        text += strspn(text, " \t");
        m_content_length = atol(text);
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        m_host = text;
    }
    else if (strncasecmp(text, "Content-Disposition:", 20) == 0)
    {

    }
    else
    {
        LOG_INFO("oop!unknow header: %s", text);
    }
    return NO_REQUEST;
}

Http::HTTP_CODE Http::parse_request_line(char *text)
{
    //在HTTP请求头中，\t用于分隔请求头字段名和字段值
    // GET /xxx.jpg HTTP/1.1
    m_url = strpbrk(text, " \t");
    if (!m_url)
    {
        return BAD_REQUEST;
    }

    *m_url++ = '\0';
    char *method = text; //GET

    //strcasecmp，比较字符串是否相同
    if (strcasecmp(method, "GET") == 0)
    {
        m_method = GET;
    }
    else if(strcasecmp(method, "POST") == 0)
    {
        m_method = POST;
        cgi = 1;
    }
    else
        return BAD_REQUEST;

    //函数strspn()返回m_url的初始段的长度，该初始段完全由包含在“\t”中的字符组成
    m_url += strspn(m_url, " \t"); //_/_http/1.1
    m_version = strpbrk(m_url, " \t"); //_http/1.1

    if (!m_version)
        return BAD_REQUEST;

    *m_version++ = '\0';
    m_version += strspn(m_version, " \t"); //http/1.1

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
        return Http::BAD_REQUEST;
    //当url为/时，显示判断界面
    if (strlen(m_url) == 1)
        strcat(m_url, "index.html"); //m_url = m_url + index.html
    m_check_state = CHECK_STATE_HEADER;
    return Http::NO_REQUEST;
}

//GET / HTTP/1.1\r\nHost: www.baidu.com.cn\r\nConnection: close\r\n\r\n
Http::LINE_STATUS Http::parse_line()
{
    char temp = '0';
    if (m_check_state == CHECK_STATE_CONTENT) {
        if (m_read_idx >= m_content_length + m_checked_idx) {
            return LINE_OK;
        }
        else {
            return LINE_OPEN;
        }
    }

    for(; m_checked_idx < m_read_idx; ++m_checked_idx)
    {
        temp = m_read_buf[m_checked_idx];
        if(temp == '\r')
        {
            if((m_checked_idx + 1) == m_read_idx)
            {
                return LINE_OPEN;
            }
            else if(m_read_buf[m_checked_idx + 1] == '\n')
            {
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if(temp == '\n')
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

bool Http::add_status_line(int status, const char *title)
{
    //http/1.1,200,OK
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool Http::add_response(const char *format, ...)
{
    if (m_write_idx >= WRITE_BUFFER_SIZE)
        return false;

    va_list arg_list; //可变参数列表
    va_start(arg_list, format); //将变量arg_list初始化为传入参数
    //将数据format从可变参数列表写入缓冲区写，返回写入数据的长度
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);

    if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx))
    {
        //
        va_end(arg_list);
        return false;
    }

    m_write_idx += len;
    va_end(arg_list);

//    LOG_INFO("request:%s", m_write_buf);

    return true;
}

bool Http::add_headers(int content_len)
{
    return add_content_length(content_len) && add_linger() &&
           add_blank_line();
}

bool Http::add_content_length(int content_len)
{
    return add_response("Content-Length:%d\r\n", content_len);
}

bool Http::add_linger()
{
    return add_response("Connection:%s\r\n", (m_linger == true) ? "keep-alive" : "close");
}

bool Http::add_blank_line()
{
    return add_response("%s", "\r\n");
}

//从内核时间表删除描述符
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void Http::close_conn(bool real_close)
{
    if(real_close && (m_sockfd != -1))
    {
        removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        m_user_count--;
    }
}

bool Http::add_content(const char *content)
{
    return add_response("%s", content);
}

Http::HTTP_CODE Http::do_request()
{
    strcpy(m_real_file, m_root);
    int len = strlen(m_root);

    const char *p = strrchr(m_url, '/');

    //处理cgi
    if (cgi == 1 && (*(p + 1) == '2' || *(p + 1) == '3'))
    {
        //根据标志判断是登录检测还是注册检测
        char flag = m_url[1];
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/");
        strcat(m_url_real, m_url + 2);
        strncpy(m_real_file + len, m_url_real, FILENAME_LEN - len - 1);
        free(m_url_real);

        //将用户名和密码提取出来
        //user=123&passwd=123
        char name[100], password[100];
        int i;
        for(i = 5; m_string[i] != '&'; ++i)
            name[i - 5] = m_string[i];
        name[i - 5] = '\0';

        int j = 0;
        for(i = i + 10; m_string[i] != '\0'; ++i, ++j)
            password[j] = m_string[i];
        password[j] = '\0';

        if(*(p + 1) == '3')
        {
            //如果是注册，先检测数据库中是否有重名的
            //没有重名的，进行增加数据
            char *sql_insert = (char *)malloc(sizeof(char) * 200);

            {
                strcpy(sql_insert, "INSERT INTO user(username, password,create_time) VALUES(");
            }
            strcat(sql_insert, "'");
            strcat(sql_insert, name);
            strcat(sql_insert, "', '");
            strcat(sql_insert, password);
            strcat(sql_insert, "',now())");

            if(users.find(name) == users.end())
            {
                m_mutex.lock();
                int res = mysql_query(mysql, sql_insert);
                users.insert(std::pair<std::string, std::string>(name, password));
                m_mutex.unlock();

                if(!res)
                    strcpy(m_url, "/log.html");
                else
                    strcpy(m_url, "/registerError.html");
            }
            else
                strcpy(m_url, "/registerError.html");
        }
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        else if(*(p + 1) == '2')
        {
            if(users.find(name) != users.end() && users[name] == password)
                strcpy(m_url, "/welcome.html");
            else
                strcpy(m_url, "/logError.html");
        }
    }

    if(*(p + 1) == '0')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/register.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if(*(p + 1) == '1')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/log.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if(*(p + 1) == '5')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/picture.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if(*(p + 1) == '6')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/video.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if(*(p + 1) == '7')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/fans.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if(*(p + 1) == '8')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 1024);
        strcpy(m_url_real, "/FileList.html");
//        strcpy(m_url_real, "/upload.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if(*(p + 1) == 'u')
    {
        std::string str(m_string);
        int start = str.find("filename=\"") + 10;
        int end = str.find("\"", start);
        m_filename = str.substr(start, end - start);

        std::ofstream file("./SaveFile/" + m_filename);
        std::istringstream ss(str);
        std::string line;
        for (int i = 0; i < 5; i++)
            getline(ss, line);
        if(file.is_open())
        {
            file << line;
            file.close();
        }
        else
        {
            file.close();
            return FORBIDDEN_REQUEST;
        }

        return SUCCESS_JSON;
    }
    else if(*(p + 1) == 'd')
    {

    }
    else
        strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1); //拼接

    if(stat(m_real_file, &m_file_stat) < 0) //获取文件信息
    {
//        LOG_ERROR("%s: %s", m_real_file, "no_found");
        return NO_RESOURCE;
    }

    if(!(m_file_stat.st_mode & S_IROTH)) //文件可读
    {
//        LOG_ERROR("%s: %s", m_real_file, "cant_read");
        return FORBIDDEN_REQUEST;
    }

    if(S_ISDIR(m_file_stat.st_mode)) //是目录不是文件
    {
//        LOG_ERROR("%s: %s", m_real_file, "is_dir");
        return BAD_REQUEST;
    }

    int fd = open(m_real_file, O_RDONLY);
    m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

void Http::unmap()
{
    if(m_file_address)
    {
        //将m_file_address映射的内存区域解除映射。
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = 0;
    }
}