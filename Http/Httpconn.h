#ifndef WEBSERVER_HTTPCONN_H
#define WEBSERVER_HTTPCONN_H

#include "../Log/Log.h"
#include "../Timer/Timer.h"

class Http {
public:
    Http();
    ~Http();

    static int m_epollfd;
    static int m_user_count;

private:
    int m_state;
    int improv;

};


#endif //WEBSERVER_HTTPCONN_H
