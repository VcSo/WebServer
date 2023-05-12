cmake_minimum_required(VERSION 3.16)

project(serverone)

add_compile_options(-std=c++11 -O3 -W -Wall)
#add_compile_options(-std=c++11 -g -W -Wall)

include_directories(
        Server
        Log
        Lock
        Sql
        Http
        Pool
        Timer
)

add_executable(serverone
        Server/Server.cpp
        Http/Httpconn.cpp
        Log/Log.cpp
        Sql/connSql.cpp
        Timer/Timer.cpp
        Lock/Locker.cpp
        Pool/ThreadPool.hpp
        main.cpp)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} -pthread -lmysqlclient)