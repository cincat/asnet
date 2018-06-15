#ifndef LOG_H
#define LOG_H


#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>

#include <iostream>
#include <string>

#include <log_stream.h>

namespace asnet {

enum LOG_LEVEL{
    INFO,
    ERROR
    // DEBUG,
    // FATAL
};

class Log {

public:
    Log(LOG_LEVEL);
    
    template <typename T>
    Log &operator<<(T n) {
        item_.append(std::to_string(n));
        return *this;
    }

    Log &operator<<(const char *s) {
        item_.append(std::string(s));
        return *this;
    }

    Log &operator<<(char *s) {
        item_.append(std::string(s));
        return *this;
    }
    Log &operator<<(const std::string &s) {
        item_.append(s);
        return *this;
    }
    ~Log() {
        log_stream_singleton_.getInstance().append(item_);
    }
    // static Log getLog(LOG_LEVEL level) {
    //     return Log(level);
    // }
private:
    // ofstream log_stream_;
    // char prefix[64];
    std::string item_;
    LogStreamSingleton log_stream_singleton_;
};
#define LOG_INFO asnet::Log(asnet::LOG_LEVEL::INFO)
#define LOG_ERROR asnet::Log(asnet::LOG_LEVEL::ERROR)

// #define LOG_FATAL Log(FATAL)
// #define LOG_DEBUG Log(DEBUG)

} //end of namespace asent

// #define LOG_INFO asnet::Log(asnet::LOG_LEVEL::INFO)
// #define LOG_ERROR asnet::Log(asnet::LOG_LEVEL::ERROR)
#endif