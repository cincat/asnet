#ifndef LOG_STREAM_H
#define LOG_STREAM_H

#include <pthread.h>

#include <string>
#include <memory>
#include <deque>

#include <mutex.h>
#include <fix_buffer.h>
#include <block_queue.h>
#include <condition.h>

namespace asnet {

class LogStream {
public:
    friend class LogStreamSingleton;
    void append(const std::string &);
    void flush();
    void doFlush();
private:
    int fd_;
    Mutex mutex_;
    Condition condition_;
    FixBuffer *primeBuffer_;
    std::deque<FixBuffer *> work_list_;
    std::deque<FixBuffer *> free_list_;
    pthread_t log_thread_;

    static const int kInterval = 3;
    LogStream();
    LogStream(const std::string &);
    ~LogStream() {flush();}
    
    // std::unique_ptr<FixBuffer> swapBuffer_;
    
};

class LogStreamSingleton {
public:
    LogStream &getInstance() {
        static LogStream stream;
        return stream;
    }
};

}// end of namespace asnet

#endif