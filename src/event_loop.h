#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <set>
#include <functional>
#include <vector>
// #include <stream.h>

#include <sys/epoll.h>

// #include <log.h>

#include <mutex.h>

namespace asnet {

class Service;
class Stream;
//bool streamComp(Stream *, Stream *);
// struct epoll_event;

class EventLoop {
public:

    EventLoop();

    EventLoop(const EventLoop &) = delete;
    EventLoop& operator=(const EventLoop &) = delete;

    void run() ;
    Stream* newStream();
    Stream* newStream(int fd);
    void setService(Service *service) {service_ = service;}
private:

    long long getBlockTime();
    void registerStreamEvents();
    void handleStreamEvents(long);
    void handleClosedEvents();
    void handleTimeoutEvents();
    void unregistStream(Stream *);

    std::set<Stream*, std::function<bool(Stream*, Stream*)>> streams_;
    std::vector<Stream*> stream_buffer_; 
    const static int kEventNum = 1000;
    Mutex mutex_;
    int efd_;
    Service *service_;
    // std::vector<struct epoll_event> epoll_event_list_;
};

}// end of asnet namespace

#endif