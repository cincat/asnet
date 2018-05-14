
#include <time.h>
// #include <sys/epoll.h>

#include <EventLoop.h>
#include <stream.h>

namespace asnet {
    bool streamComp(Stream *, Stream*);
    EventLoop::EventLoop(): streams_(std::bind(streamComp, std::placeholders::_1, std::placeholders::_2)){

    }

    void EventLoop::run() {
        Stream *stream = nullptr;
        if (streams_.empty() == false) {
            stream = *streams_.begin();
        }
        if (stream == nullptr) return;

        long block_time = stream->getExpiredTime();
        int efd = ::epoll_create(1);
        // fix me
        if (efd < 0) {
            return;
        }

        for (auto item : streams_) {
            struct epoll_event event;
            event.data.ptr = item;
            epoll_ctl(efd, EPOLL_CTL_ADD, item->getFd(), &event);
        }
        
        epoll_wait(efd, &*epoll_event_list_.begin(), epoll_event_list_.size(), block_time);

        for (auto event : epoll_event_list_) {
            if (event.events == 0) continue;
            stream = static_cast<Stream *>(event.data.ptr);

            if ((event.events == EPOLLIN | EPOLLPRI) && stream->getState == Stream::State::LISTENNING) {
                stream->setState(Stream::State::CONNECTED);
            }
        }
    }
    Stream* EventLoop::newStream() {
        Stream *stream = new Stream();
        streams_.insert(stream);
        struct epoll_event event;
        event.data.ptr = stream;
        epoll_event_list_.push_back(event);
        return stream;
    }

}