#include <set>
#include <functional>
#include <vector>
// #include <stream.h>

#include <sys/epoll.h>

namespace asnet {

class Stream;
// struct epoll_event;

class EventLoop {
public:

    EventLoop();

    EventLoop(const EventLoop &) = delete;
    EventLoop& operator=(const EventLoop &) = delete;

    void run() ;
    Stream* newStream();
private:
    std::set<Stream*, std::function<bool(Stream*, Stream*)>> streams_;
    std::vector<struct epoll_event> epoll_event_list_;
};

class StreamPtrHeap {

};

}// end of asnet namespace