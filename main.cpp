//
// Created by Vcvc on 2021/09-07.
//

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

    Server server(port, "localhost", "root", "Aa1248800211", "serverm",
                    true, 1, 1, 8, 10, 1, 1); //启用 友善关闭 lt/et sql thread actor 异步

    server.set_log("./Savelog/");
    server.setsql();
//    server.threadpool();
//    server.trig_mode();
//    server.event_listen();
//
//    server.Start();

    sleep(3);
    std::cout << "Server close" << std::endl;

    return 0;
}

