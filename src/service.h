#ifndef SERVICE_H
#define SERVICE_H

#include <event_loop.h>
#include <thread_pool.h>

namespace asnet {

class Service {
public:
    Service(int num);
    Stream* newStream() {return loop_.newInternalStream();}
    ThreadPool *getThreadPool() {return &threads_;}
    void start();
private:
    EventLoop loop_;
    ThreadPool threads_;
};

}// end of namespace asnet

#endif
