#ifndef WEBSERVER_HTTPCONN_H
#define WEBSERVER_HTTPCONN_H

#include "../Log/Log.h"
#include "../Timer/Timer.h"

class Http {
public:
    Http();
    ~Http();

private:
    int m_state;
    int improv;

};


#endif //WEBSERVER_HTTPCONN_H
