#include <iostream>

#include "./Server/Server.h"

int main(int argc, char **argv)
{
    std::cout << "main start" << std::endl;
    int port = 20999;
    if(argc > 2)
    {
        port = atoi(argv[1]);
    }

    std::cout << "http://101.132.159.69:" << port << std::endl;

/*
0，表示使用LT + LT
1，表示使用LT + ET
2，表示使用ET + LT
3，表示使用ET + ET

actor 1 = re 0=pr
*/
    Server server(port, "localhost", "root", "zxz456123", "serverm",
                  true, 1, 1, 8, 10, 1, 1); //启用 友善关闭 lt/et sql thread actor 异步

    server.set_log("./Savelog/");
    server.setsql();
    server.threadpool();
    server.trig_mode();
    server.event_listen();
    server.Start();

    std::cout << "Server close" << std::endl;

    return 0;
}

//create table stu(
//        id int(10) not null auto_increment primary key,
//        nickname char(16) default null,
//        name char(20) not null,
//        age int(2) default null,
//        sex char(1) not null,
//        phone int(11) not null
//        create_time datetime
//        )ENGINE=InnoDB default charset=utf8;