#include "Log.h"

Log::Log() {}

Log::~Log() {}

void Log::init(std::string path, bool uselog, int log_buf_size, int split_line, int max_queue_size)
{
    if(max_queue_size > 0)
    {
        m_is_async = true;

    }
}
