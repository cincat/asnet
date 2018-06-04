#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <set>
#include <functional>
#include <vector>
// #include <stream.h>

#include <sys/epoll.h>

// #include <log.h>

namespace asnet {

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
private:

    long long getBlockTime();
    void registerStreamEvents();
    void handleStreamEvents(long);
    void handleClosedEvents();

    std::set<Stream*, std::function<bool(Stream*, Stream*)>> streams_;
    std::vector<Stream*> stream_buffer_; 
    const static int kEventNum = 1000;
    int efd_;
    // std::vector<struct epoll_event> epoll_event_list_;
};

}// end of asnet namespace

#endif