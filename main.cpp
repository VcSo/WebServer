#include <iostream>
#include <getopt.h>

#include "./Server/Server.h"

static struct option opts[] = {
        {"ip", required_argument, NULL, 'i'},
        {"port", required_argument, NULL, 'p'},
        {"sql_username", required_argument, NULL, 'u'},
        {"sql_passwrod", required_argument, NULL, 'w'},
        {"sql_database", required_argument, NULL, 'd'},
        {"use_log", required_argument, NULL, 'g'},
        {"lingermode", required_argument, NULL, 'l'},
        {"et", required_argument, NULL, 'e'},
        {"sql_threadnum", required_argument, NULL, 's'},
        {"thread_num", required_argument, NULL, 't'},
        {"actor_mode", required_argument, NULL, 'a'},
        {"async", required_argument, NULL, 'c'},
        {0, 0, 0, 0}
};

int main(int argc, char **argv)
{
    std::string ip = "";
    int port = 20999;
    std::string sql_username = "root";
    std::string sql_password = "";
    std::string sql_database = "serverm";
    bool use_log = true;
    int lingermode = 0;

    /*
    et_init:
    0，表示使用LT + LT
    1，表示使用LT + ET
    2，表示使用ET + LT
    3，表示使用ET + ET
    */
    int et = 3;
    int sql_threadnum = 8;
    int thread_num = 10;
    /*
     * actor_mode:
     * 0: proactor
     * 1: reactor
     */
    int actor_mode = 0;
    int async = 1;

    int opt = 0;
    int optionindex = 0;

//  ip, port, sql_username, sql_password, sql_database, use_log, lingermode, et, sqlthreadnum, threadnum, actor_mode, async
    while((opt = getopt_long(argc, argv, "i:p:u:w:d:g:l:e:s:t:a:c:", opts, &optionindex)) != -1)
    {
        switch(opt)
        {
            case 'i':
                ip = optarg;
                break;
            case 'p':
                port = atoi(optarg);;
                break;
            case 'u':
                sql_username = optarg;
                break;
            case 'w':
                sql_password = optarg;
                break;
            case 'd':
                sql_database = optarg;
                break;
            case 'g':
                if(*optarg == '0')
                    use_log = false;
                else
                    use_log = true;
                break;
            case 'l':
                lingermode = atoi(optarg);
                break;
            case 'e':
                et = atoi(optarg);
                break;
            case 's':
                sql_threadnum = atoi(optarg);
                break;
            case 't':
                thread_num = atoi(optarg);
                break;
            case 'a':
                actor_mode = atoi(optarg);
                break;
            case 'c':
                async = atoi(optarg);
                break;
            default:
                ;
        }
    }
    std::cout << "http://" << ip << ":" << port << std::endl;

    Server server(ip, port, "localhost", sql_username, sql_password, sql_database,
                  use_log, lingermode, et, sql_threadnum, thread_num, actor_mode, async); //日志 友善关闭 lt/et sql thread actor 异步

    server.set_log("./Savelog/");
    server.setsql();
    server.threadpool();
    server.trig_mode(); //触发模式
    server.event_listen();
    server.Start();

    std::cout << "Server close" << std::endl;

    return 0;
}