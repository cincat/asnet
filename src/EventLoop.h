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

    EventLoop(): streams_(streamComp){};

    EventLoop(const EventLoop &) = delete;
    EventLoop& operator=(const EventLoop &) = delete;

    void run() ;
    Stream* newStream();
    Stream* newStream(int fd);
private:

    long long getBlockTime();
    void registerStreamEvent(int);
    void handleStreamEvent(int, long);

    std::set<Stream*, std::function<bool(Stream*, Stream*)>> streams_;
    std::vector<Stream*> stream_buffer_;
    const static int kEventNum = 1000;
    // int efd_;
    // std::vector<struct epoll_event> epoll_event_list_;
};

class StreamPtrHeap {

};

}// end of asnet namespace