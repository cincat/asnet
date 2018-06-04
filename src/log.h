#ifndef LOG_H
#define LOG_H


#include <string.h>
#include <iostream>
#include <sys/time.h>
#include <stdlib.h>

namespace asnet {

enum LOG_LEVEL{
    INFO,
    ERROR,
    DEBUG,
    FATAL
};

class Log {

public:
    Log(LOG_LEVEL level) {
        switch(level) {
            case INFO: ::strcpy(prefix, "[INFO]: "); break;
            case ERROR: ::strcpy(prefix, "[ERROR]: "); break;
            case DEBUG: ::strcpy(prefix, "[DEBUG]: "); break;
        }
        strcat(prefix, getenv("_"));
        strcat(prefix, " ");
    }

    template <typename T>
    std::ostream& operator <<(const T &s) {
        // struct timeval val;
        // gettimeofday(&val, nullptr);
        std::cerr << prefix << s;
        if (::strlen(prefix) > 1) {
            ::memset(prefix, 0, sizeof(prefix));
            // prefix[0] = ' ';
        }
        return std::cerr;
    }

    static Log getLog(LOG_LEVEL level) {
        return Log(level);
    }
private:
    // ofstream log_stream_;
    char prefix[32];
};

#define LOG_INFO Log::getLog(INFO)
#define LOG_ERROR Log::getLog(ERROR)
#define LOG_FATAL Log::getLog(FATAL)
#define LOG_DEBUG Log::getLog(DEBUG)

} //end of namespace asent

#endif