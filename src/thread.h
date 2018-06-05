#ifndef THREAD_H
#define THREAD_H

#include <event_loop.h>
#include <pthread.h>

namespace asnet {

class Thread {
public:
    Thread();
    void run();
    EventLoop *getEventLoop();

    Thread(Thread &) = delete;
    Thread& operator =(Thread &) = delete;
private:
    EventLoop loop_;
    pthread_t thread_;
};

}// end of namespace asnet

#endif