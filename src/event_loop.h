#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <set>
#include <functional>
#include <vector>
// #include <stream.h>

#include <sys/epoll.h>

// #include <log.h>

#include <mutex.h>
#include <memory_pool.h>
#include <block_queue.h>
#include <stream.h>
#include <connection.h>

namespace asnet {

class Service;
// class Stream;
//bool streamComp(Stream *, Stream *);
// struct epoll_event;

class EventLoop {
public:

    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop &) = delete;
    EventLoop& operator=(const EventLoop &) = delete;

    void run() ;
    Stream *newStream();
    Stream *newStream(int fd);
    Stream *newInternalStream();
    void setService(Service *service) {service_ = service;}
    void appendCallback(std::pair<Stream::Callback, Connection>);
private:

    void createEvent();
    void invokeCallbacks();
    long long getBlockTime();
    void registerStreamEvents();
    void handleStreamEvents(long);
    void handleClosedEvents();
    void handleTimeoutEvents();
    void unregistStream(Stream *);

    std::set<Stream*, std::function<bool(Stream*, Stream*)>> streams_;
    // std::vector<Stream*> stream_buffer_; 
    BlockQueue<Stream *> stream_buffer_;
    BlockQueue<std::pair<Stream::Callback, Connection>> callback_buffer_;
    const static int kEventNum = 1000;
    Mutex mutex_;
    int efd_;
    int event_fd_;
    Service *service_;
    MemoryPool pool_;
    // std::vector<struct epoll_event> epoll_event_list_;
};

}// end of asnet namespace

#endif