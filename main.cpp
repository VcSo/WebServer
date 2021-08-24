//
// Created by Vcvc on 2021/8/20.
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

    Server server(port, "localhost", "root", "vcvc", "server",
                    true, 1, 1, 8, 10, 1, 1); //启用 友善关闭 lt/et sql thread actor 异步

    server.setsql();
    server.setlog("./savelog/");

    return 0;
}

