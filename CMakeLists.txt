cmake_minimum_required(VERSION 3.16)
project(WebServer)

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_STANDARD 11)

add_executable(WebServer main.cpp
        Server/Server.cpp
        Http/Httpconn.cpp
        ThreadPool/ThreadPool.hpp
        log/log.cpp
                )