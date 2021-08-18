#ifndef WEBSERVER_HTTPCONN_H
#define WEBSERVER_HTTPCONN_H

#include <cstring>

class Httpconn {
public:
    Httpconn();
    ~Httpconn();


    //https://blog.csdn.net/sevenjoin/article/details/81772792 为什么static成员变量一定要在类外初始化
    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;

private:

};

#endif //WEBSERVER_HTTPCONN_H
