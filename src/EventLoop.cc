
#include <time.h>
// #include <sys/epoll.h>

#include <EventLoop.h>
#include <stream.h>

namespace asnet {
    bool streamComp(Stream *, Stream*);
    EventLoop::EventLoop(): streams_(std::bind(streamComp, std::placeholders::_1, std::placeholders::_2)){
        // efd_ = epoll_create(1);
        // // fix me: add some log information
        // if (efd_ < 0) {
        //     return;
        // }
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

        for (auto stream_ptr : stream_buffer_) {
            if (stream_ptr->getState() == Stream::State::CONNECTING) {
                struct epoll_event event;
                event.events = EPOLLOUT;
                event.data.ptr = stream_ptr;
                epoll_ctl(efd, EPOLL_CTL_ADD, stream_ptr->getFd(), &event);
            }
            else if (stream_ptr->getState() == Stream::State::LISTENNING) {
                struct epoll_event event;
                event.events = EPOLLIN;
                event.data.ptr = stream_ptr;
                epoll_ctl(efd, EPOLL_CTL_ADD, stream_ptr->getFd(), &event);
            }
            else if (stream_ptr->getState() == Stream::State::CONNECTED) {
                struct epoll_event event;
                event.events = EPOLLIN;
                if (stream_ptr->writable()) event.events |= EPOLLOUT;
                event.data.ptr = stream_ptr;
                epoll_ctl(efd, EPOLL_CTL_ADD, stream_ptr->getFd(), &event);
            }
            else if (stream_ptr->getState() == Stream::State::CLOSING) {
                struct epoll_event event;
                event.events = EPOLLOUT;
                event.data.ptr = stream_ptr;
                epoll_ctl(efd, EPOLL_CTL_ADD, stream_ptr->getFd(), &event);
            }
            streams_.insert(stream_ptr);
        }
        stream_buffer_.clear();

        struct epoll_event event_list[kEventNum];
        int nfds;
        nfds = epoll_wait(efd, event_list, kEventNum, block_time);
        // fix me: add log information
        if (nfds < 0) {
            return ;
        }

        for (int i = 0; i < nfds; i++) {

        }
    }
    Stream* EventLoop::newStream() {
        Stream *stream = new Stream();
        stream_buffer_.push_back(stream);
        // streams_.insert(stream);
        // struct epoll_event event;
        // event.data.ptr = stream;
        // epoll_event_list_.push_back(event);
        return stream;
    }

}