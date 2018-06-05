#ifndef THREAD_H
#define THREAD_H

#include <event_loop.h>
#include <pthread.h>
#include <mutex.h>
#include <condition.h>

namespace asnet {

class Thread {
public:
    Thread();
    void run();
    EventLoop *getEventLoop();

    Thread(Thread &) = delete;
    Thread& operator =(Thread &) = delete;
    
    void run();
    void threadFunc();
private:
    EventLoop loop_;
    pthread_t thread_;
    Mutex mutex_;
    Condition cond_;
};

}// end of namespace asnet

#endif